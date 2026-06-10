document.addEventListener('DOMContentLoaded', function() {
    const currentTime = new Date().toLocaleTimeString('pl-PL', { timeZone: 'Europe/Warsaw' });
    const hour = parseInt(currentTime.split(':')[0], 10);
    const customBackground = document.querySelector('.custom-background');
  
    if (hour >= 6 && hour < 11) {
      customBackground.style.backgroundImage = 'url("assets/1.jpg")';
    } else if (hour >= 11 && hour < 18) {
      customBackground.style.backgroundImage = 'url("assets/2.jpg")';
    } else if (hour >= 18 && hour < 21) {
      customBackground.style.backgroundImage = 'url("assets/3.jpg")';
    } else {
      customBackground.style.backgroundImage = 'url("assets/4.jpg")';
    }
});