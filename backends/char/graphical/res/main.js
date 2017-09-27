var term;
var terminalContainer = document.getElementById('terminal-container');

createTerminal();

window.addEventListener('resize', function () {
    term.fit();
});

function createTerminal() {
    // Clean terminal
    while (terminalContainer.children.length) {
        terminalContainer.removeChild(terminalContainer.children[0]);
    }

    term = new Terminal({
        cursorBlink: false,
        scrollback: 1000,
        tabStopWidth: 8
    });

    terminalContainer.style.position = "fixed";
    terminalContainer.style.top = 0;
    terminalContainer.style.bottom = 0;
    terminalContainer.style.left = 0;
    terminalContainer.style.right = 0;
    terminalContainer.style.width = "auto";
    terminalContainer.style.height = "auto";

    term.open(terminalContainer, true);
    term.fit();

    term.on('key', function (key, ev) {
        bridge.callback(btoa(key));
    });

    term.on('paste', function (data, ev) {
        bridge.callback(btoa(data));
    });
}

function writeToTerminal(data) {
    term.write(atob(data));
}

