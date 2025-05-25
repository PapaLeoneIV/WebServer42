function sayHello() {
    const arr = [
      "You clicked the button! JavaScript is working",
      "Progetto Troppo Difficile",
      "Ripigliatevi",
      "Ur Mom"
    ];

    const paragraphs = document.getElementsByClassName('message');

    for (var i = 0; i < paragraphs.length; i++) {
        // Pick random index between 0 and arr.length - 1
        var randomIndex = Math.floor(Math.random() * arr.length);
        paragraphs[i].textContent = arr[randomIndex];
    }
}