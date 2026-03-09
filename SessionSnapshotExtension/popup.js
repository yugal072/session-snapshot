// popup.js

document.addEventListener('DOMContentLoaded', () => {
    const saveBtn = document.getElementById('save-btn');
    const sessionNameInput = document.getElementById('session-name');
    const sessionsList = document.getElementById('sessions-list');
    const alertsContainer = document.getElementById('alerts');

    // Load and display sessions
    function loadSessions() {
        chrome.storage.local.get(['savedSessions'], (result) => {
            const sessions = result.savedSessions || [];
            sessionsList.innerHTML = '';

            if (sessions.length === 0) {
                sessionsList.innerHTML = '<div class="empty-state">No saved sessions yet.</div>';
                return;
            }

            sessions.forEach(session => {
                const card = document.createElement('div');
                card.className = 'session-card';

                const infoDiv = document.createElement('div');
                infoDiv.className = 'session-info';
                infoDiv.title = "Click to restore session";

                const dateStr = new Date(session.timestamp).toLocaleString();

                infoDiv.innerHTML = `
          <div class="session-name">${escapeHTML(session.name)}</div>
          <div class="session-meta">${session.tabs.length} tabs • ${dateStr}</div>
        `;

                // Restore session on clicking the info area
                infoDiv.addEventListener('click', () => {
                    chrome.runtime.sendMessage({ action: 'restoreSession', sessionId: session.id }, (response) => {
                        if (response && response.success) {
                            showAlert('Session restoring...', 'success');
                        } else {
                            showAlert(response ? response.message : 'Error restoring session', 'error');
                        }
                    });
                });

                const actionsDiv = document.createElement('div');
                actionsDiv.className = 'session-actions';

                const deleteBtn = document.createElement('button');
                deleteBtn.className = 'icon-btn btn-delete';
                deleteBtn.innerHTML = '×';
                deleteBtn.title = 'Delete Session';
                deleteBtn.addEventListener('click', (e) => {
                    e.stopPropagation();
                    if (confirm('Are you sure you want to delete this session?')) {
                        chrome.runtime.sendMessage({ action: 'deleteSession', sessionId: session.id }, (response) => {
                            loadSessions(); // Reload list after deletion
                        });
                    }
                });

                actionsDiv.appendChild(deleteBtn);
                card.appendChild(infoDiv);
                card.appendChild(actionsDiv);
                sessionsList.appendChild(card);
            });
        });
    }

    // Handle save button click
    saveBtn.addEventListener('click', () => {
        const sessionName = sessionNameInput.value.trim();
        saveBtn.disabled = true;
        saveBtn.textContent = 'Saving...';

        // Instead of sending message via runtime (which needs background script to read current window),
        // Background script might get the *background extension window*.
        // Using chrome.tabs.query({currentWindow: true}) inside popup.js directly is better for "current window".
        // Alternatively, background script chrome.tabs.query({currentWindow: true}) from an onMessage might query the active window correctly. 
        // Let's use the background script message passing. Be aware currentWindow in background script might sometimes resolve to the popup itself if it's considered a window, but typically background script uses chrome.windows.getLastFocused.

        // To be safe, let's just query tabs *here* and pass them to background.js? 
        // Actually, background.js runtime listener is fine, but in manifest v3 we can just query here.
        // Let's send a message and see.
        chrome.runtime.sendMessage({ action: 'saveSession', sessionName: sessionName }, (response) => {
            saveBtn.disabled = false;
            saveBtn.textContent = 'Save Current Session';

            if (response && response.success) {
                sessionNameInput.value = '';
                showAlert(response.message, 'success');
                loadSessions();
            } else {
                showAlert(response ? response.message : 'Error saving session', 'error');
            }
        });
    });

    function showAlert(msg, type) {
        alertsContainer.innerHTML = `<div class="alert alert-${type}">${escapeHTML(msg)}</div>`;
        setTimeout(() => {
            alertsContainer.innerHTML = '';
        }, 4000);
    }

    function escapeHTML(str) {
        return str.replace(/[&<>'"]/g,
            tag => ({
                '&': '&amp;',
                '<': '&lt;',
                '>': '&gt;',
                "'": '&#39;',
                '"': '&quot;'
            }[tag] || tag)
        );
    }

    // Initial load
    loadSessions();
});
