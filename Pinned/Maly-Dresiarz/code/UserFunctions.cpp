#include "UserFunctions.h"
#include "User.h"

void showOff(User& user) {
	user.respect += 1;
}
void work(User& user) {
	user.cash += 2;
}
void addExtortion(User& user) {
	user.cash -= 4;
	user.extortion++;
}
void collectExtortion(User& user) {
	for (int i = 0; i < user.extortion; i++)
		user.cash++;
}
void stealBMW(User& user) {
	user.BMW++;
}
void useBMW(User& user) {

	for (int i = 0; i < user.BMW; i++) {
		if (user.cash >= 2) {
			user.cash -= 2;
			user.respect += 3;
		}
		// sprawdzenie czy gracz nie przekroczy wartosci == 0
		else if (user.respect == 1 && user.cash < 2) user.respect -= 1;
		else if (user.respect > 1 && user.cash < 2) user.respect -= 2;
	}
}
