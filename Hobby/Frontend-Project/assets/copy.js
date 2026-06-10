function showAlert() {
    Swal.fire({
        icon: 'success',
        title: 'Prawidłowo skopiowano IP serwera!',
        showConfirmButton: false,
        timer: 2222,
        position: 'center'
    });
}

document.addEventListener('DOMContentLoaded', function() {
    const inviterUpperButtons = document.querySelectorAll('.inviter-upper');
    inviterUpperButtons.forEach(button => {
        button.addEventListener('click', async function() {
            const textToCopy = 'smyf.pl';
            try {
                await navigator.clipboard.writeText(textToCopy);
                showAlert();
            } catch (err) {
                console.error('Błąd podczas kopiowania do schowka:', err);
            }
        });
    });
});