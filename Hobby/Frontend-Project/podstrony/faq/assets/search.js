document.addEventListener('DOMContentLoaded', function () {
    const searchInput = document.querySelector('.bottom input');
    const sections = document.querySelectorAll('.section');
    const searchResult = document.querySelector('.search-result');
    const searchResultCount = document.querySelector('.search-count');
    const searchImage = document.querySelector('.image-search');
    const footer = document.querySelector('.footer');
    
  
    function hideSection(section) {
      section.style.display = 'none';
    }
  
    function showSection(section) {
      section.style.display = 'grid';
    }
  
    function textContainsKeyword(text, keyword) {
      return text.toLowerCase().includes(keyword);
    }
  
    function handleSearch() {
      const keyword = searchInput.value.toLowerCase().trim();
      let resultCount = 0;
  
      if (!keyword) {
        sections.forEach(showSection);
        searchResult.classList.remove('shifted');
      } else {
        sections.forEach(section => {
          const questionText = section.querySelector('.question').textContent;
          if (textContainsKeyword(questionText, keyword)) {
            showSection(section);
            resultCount++;
          } else {
            hideSection(section);
          }
        });
  
        if (resultCount == 0) {
          searchResultCount.textContent = `Brak wyników dla słowa klucz: ${keyword}.`;
        }
        if (resultCount == 1) {
          searchResultCount.textContent = `Znaleziono ${resultCount} wynik dla słowa klucz: ${keyword}.`;
        }
        if (resultCount > 1 && resultCount < 5) {
          searchResultCount.textContent = `Znaleziono ${resultCount} wyniki dla słowa klucz: ${keyword}.`;
        }
        if (resultCount > 4) {
          searchResultCount.textContent = `Znaleziono ${resultCount} wyników dla słowa klucz: ${keyword}.`;
        }
        searchResult.classList.add('shifted');
      }

      // Sprawdzanie szerokości okna i ukrywanie/pokazywanie .footer
      if(window.innerHeight >= document.body.scrollHeight) {
        footer.style.display = 'none';
      } else {
          footer.style.display = 'flex';
      }
    }

    searchImage.addEventListener('click', handleSearch);
  
    searchInput.addEventListener('keyup', function (event) {
      if (event.key === 'Enter') {
        handleSearch();
      }
    });
  
    searchResult.addEventListener('click', function () {
      searchInput.value = '';
      handleSearch();
    });
  });
  