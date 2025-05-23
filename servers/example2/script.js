function fetchTime() {
  fetch('/cgi-bin/random/time.py')
    .then(response => response.text())
    .then(data => {
      document.getElementById('time-display').textContent = data;
    })
    .catch(err => {
      document.getElementById('time-display').textContent = "Failed to get time.";
    });
}
