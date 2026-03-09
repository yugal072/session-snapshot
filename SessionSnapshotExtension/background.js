// background.js

// Function to save the current session
async function saveCurrentSession(sessionName) {
    try {
        const tabs = await chrome.tabs.query({ currentWindow: true });

        // Filter out tabs with URLs that cannot be saved (e.g., chrome://, edge://, about:blank)
        const validTabs = tabs.filter(tab => !tab.url.startsWith('chrome://') && !tab.url.startsWith('edge://') && !tab.url.startsWith('about:'));

        const sessionData = validTabs.map(tab => ({
            title: tab.title || tab.url,
            url: tab.url,
            favIconUrl: tab.favIconUrl || ''
        }));

        if (sessionData.length === 0) {
            return { success: false, message: "No valid tabs to save in this window." };
        }

        const timestamp = Date.now();
        const sessionId = `session_${timestamp}`;
        const name = sessionName || `Session ${new Date(timestamp).toLocaleString()}`;

        const newSession = {
            id: sessionId,
            name: name,
            timestamp: timestamp,
            tabs: sessionData
        };

        // Get existing sessions from storage
        const result = await chrome.storage.local.get(['savedSessions']);
        let savedSessions = result.savedSessions || [];

        // Add the new session to the beginning of the list
        savedSessions.unshift(newSession);

        // Save back to storage
        await chrome.storage.local.set({ savedSessions: savedSessions });

        return { success: true, message: `Session saved with ${sessionData.length} tabs.` };
    } catch (error) {
        console.error("Error saving session:", error);
        return { success: false, message: "Failed to save session." };
    }
}

// Function to restore a session
async function restoreSession(sessionId) {
    try {
        const result = await chrome.storage.local.get(['savedSessions']);
        const savedSessions = result.savedSessions || [];

        const sessionToRestore = savedSessions.find(s => s.id === sessionId);

        if (sessionToRestore && sessionToRestore.tabs.length > 0) {
            // Create a new window containing the tabs
            chrome.windows.create({ url: sessionToRestore.tabs.map(tab => tab.url) });
            return { success: true, message: "Session restored." };
        } else {
            return { success: false, message: "Session not found or empty." };
        }
    } catch (error) {
        console.error("Error restoring session:", error);
        return { success: false, message: "Failed to restore session." };
    }
}

// Listen for messages from the popup
chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
    if (request.action === 'saveSession') {
        saveCurrentSession(request.sessionName).then(sendResponse);
        return true; // Indicates async response
    } else if (request.action === 'restoreSession') {
        restoreSession(request.sessionId).then(sendResponse);
        return true; // Indicates async response
    } else if (request.action === 'deleteSession') {
        chrome.storage.local.get(['savedSessions']).then((result) => {
            let savedSessions = result.savedSessions || [];
            savedSessions = savedSessions.filter(s => s.id !== request.sessionId);
            chrome.storage.local.set({ savedSessions: savedSessions }).then(() => {
                sendResponse({ success: true });
            });
        });
        return true; // Async response
    }
});
