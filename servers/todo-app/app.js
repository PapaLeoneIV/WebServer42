document.addEventListener("DOMContentLoaded", () => {
  const form = document.getElementById("todo-form");
  const input = document.getElementById("todo-input");
  const list = document.getElementById("todo-list");

  let todos = JSON.parse(localStorage.getItem("todos")) || [];

  function saveTodos() {
    localStorage.setItem("todos", JSON.stringify(todos));
  }

  function renderTodos() {
    list.innerHTML = "";
    todos.forEach((todo, index) => {
      const li = document.createElement("li");
      li.className = todo.completed ? "completed" : "";

      const span = document.createElement("span");
      span.textContent = todo.text;
      span.addEventListener("click", () => {
        todos[index].completed = !todos[index].completed;
        saveTodos();
        renderTodos();
      });

      const deleteBtn = document.createElement("button");
      deleteBtn.textContent = "X";
      deleteBtn.addEventListener("click", () => {
        todos.splice(index, 1);
        saveTodos();
        renderTodos();
      });

      li.appendChild(span);
      li.appendChild(deleteBtn);
      list.appendChild(li);
    });
  }

  form.addEventListener("submit", (e) => {
    e.preventDefault();
    const text = input.value.trim();
    if (text !== "") {
      todos.push({ text, completed: false });
      saveTodos();
      renderTodos();
      input.value = "";
    }
  });

  renderTodos();
});
