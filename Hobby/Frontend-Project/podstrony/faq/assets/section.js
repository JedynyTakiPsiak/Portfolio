$(document).ready(function () {
  $('.section').click(function () {
      var $section = $(this);
      var $answer = $section.find('.answer');
      var slideDuration = 222;

      if ($answer.is(':visible')) {
          $answer.stop().slideUp(slideDuration, function () {
              $section.removeClass('active'); // Usunięcie klasy 'active' po zakończeniu animacji
          });
      } else {
          $answer.stop().slideDown(slideDuration, function () {
              $section.addClass('active'); // Dodanie klasy 'active' po zakończeniu animacji
          });
      }

      // Dodaj obrót o 180 stopni, jeśli sekcja jest aktywna
      if (this.classList.contains('active')) {
        var random = Math.random();
        if (random < 0.5) {
          $section.find('.image').css('transform', 'rotate(0deg)');
        } else {
          $section.find('.image').css('transform', 'rotate(360deg)');
        }
      } else {
        $section.find('.image').css('transform', 'rotate(180deg)');
      }
    
  });
});
