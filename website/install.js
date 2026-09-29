// Front-page install tabs: show the instructions matching the visitor's OS.
// Scripts load synchronously before <main> parses, so wait for the DOM.
document.addEventListener("DOMContentLoaded", () => {
    const tabs = document.querySelector(".os-tabs");
    if (!tabs) return;
    const panes = document.querySelectorAll(".os-pane");

    const show = (os) => {
        tabs.querySelectorAll("button").forEach((button) => {
            button.setAttribute("aria-pressed", button.getAttribute("data-os") === os ? "true" : "false");
        });
        panes.forEach((pane) => {
            pane.hidden = pane.getAttribute("data-os") !== os;
        });
    };

    tabs.addEventListener("click", (event) => {
        const button = event.target.closest("button");
        if (button) show(button.getAttribute("data-os"));
    });

    const platform = (navigator.userAgentData && navigator.userAgentData.platform) || navigator.platform || "";
    show(/win/i.test(platform) ? "windows" : "unix");
});
