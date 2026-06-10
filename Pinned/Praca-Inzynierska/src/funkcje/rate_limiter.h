#pragma once

#include <chrono>   // Date and time library
#include <thread>   // sleep_for()

// Rate-Limiter do obsługi ograniczenia przepływu zapytań do LottoAPI
// Na bazie kodu autorstwa: Erik Rigtorp <erik@rigtorp.se>
// https://github.com/rigtorp/TokenBucket/blob/master/TokenBucket.h
// Z książki Alex Xu otrzymanej od wykładowcy podczas zajęć "Aplikacje i usługi w teleinformatyce": 
// Alex Yu - System Design Interview An Insider’s Guide (2020, Independently published) - Chapter 4 - Rate Limiter
class RateLimiter {
private:
    double tokens_;             // Aktualna liczba tokenów
    const double max_tokens_;   // Maksymalna pojemność wiadra
    const double refill_rate_;  // Ile tokenów dodajemy na sekundę
    std::chrono::steady_clock::time_point last_refill_;

    void Refill() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - last_refill_).count();

        // Dodajemy tokeny do wiadra oraz ustawiamy czas na obecny, dla późniejszego odliczania
        tokens_ = min(max_tokens_, tokens_ + elapsed * refill_rate_);
        last_refill_ = now;
    }
public:
    // np. RateLimiter(10.0, 5.0) - max to 10 tokenów i odnawiamy co sekundę o 5 tokenów do wiadra
    RateLimiter(double maxTokens, double refillRate)
        :   tokens_(maxTokens),
            max_tokens_(maxTokens),
            refill_rate_(refillRate),
            last_refill_(std::chrono::steady_clock::now()) {}

    // Zabieramy 1 token przy każdym wywołaniu klasy.
    // Jeśli nie ma dostępnego tokena, czekamy aż zostanie dodany.
    void Consume(double tokens = 1.0) {
        while (true) {
            Refill();

            if (tokens_ >= tokens) {
                tokens_ -= tokens;
                return;
            }

            // Czekamy aż dojdzie kolejny token
            double tokens_needed = tokens - tokens_;
            double wait_time = tokens_needed / refill_rate_;
            std::this_thread::sleep_for(std::chrono::duration<double>(wait_time));
        }
    }

    // Funkcja do odczekania w momencie natknięcia się na HTTP Error 429 - too many request
    // Resetujemy tokeny aby na starcie restartu nie zapchać znów
    void WaitForRateLimitReset() {
        tokens_ = 0.0;
        double wait_time = max_tokens_;
        std::this_thread::sleep_for(std::chrono::duration<double>(wait_time));
        last_refill_ = std::chrono::steady_clock::now();
    }
};