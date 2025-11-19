// Sidebar droite sticky pour navigation par section
(function () {
  function createSectionSidebar() {
    // Cherche toutes les sections principales (h2)
    var headings = document.querySelectorAll(".content h2");
    if (!headings.length) return;
    var sidebar = document.createElement("nav");
    sidebar.className = "section-sidebar";
    var list = document.createElement("ul");
    list.className = "section-nav-list";
    headings.forEach(function (h2, idx) {
      var id = h2.id || "section-" + idx;
      h2.id = id;
      var li = document.createElement("li");
      var a = document.createElement("a");
      a.href = "#" + id;
      a.textContent = h2.textContent.replace(/^[^\w\d]+/, "");
      li.appendChild(a);
      list.appendChild(li);
    });
    sidebar.appendChild(list);
    document.body.appendChild(sidebar);
  }
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", createSectionSidebar);
  } else {
    createSectionSidebar();
  }
})();
