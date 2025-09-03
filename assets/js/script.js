document.addEventListener('DOMContentLoaded', () => {
    // ===== 认证和用户管理 =====
    let currentUser = null;
    
    // 登录界面元素
    const loginContainer = document.getElementById('login-container');
    const appContainer = document.getElementById('app-container');
    const usernameInput = document.getElementById('username-input');
    const loginBtn = document.getElementById('login-btn');
    const loginMessage = document.getElementById('login-message');
    const recentUsersDiv = document.getElementById('recent-users');
    
    // 用户信息栏元素
    const currentUsernameSpan = document.getElementById('current-username');
    const userProgressSpan = document.getElementById('user-progress');

    const logoutBtn = document.getElementById('logout-btn');
    
    // Main content elements
    const modeSelection = document.querySelector('.mode-selection');
    const wordDisplay = document.getElementById('word-display');
    const completionMessage = document.getElementById('completion-message');
    const welcomePanel = document.getElementById('welcome-panel');
    const listenInterface = document.getElementById('listen-interface');

    const wordList = document.getElementById('word-list');
    const submitBtn = document.getElementById('submit-btn');
    const learnModeBtn = document.getElementById('learn-mode-btn');
    const examModeBtn = document.getElementById('exam-mode-btn');
    const reviewModeBtn = document.getElementById('review-mode-btn');
    const wordReaderBtn = document.getElementById('word-reader-btn');
    const returnHomeBtn = document.getElementById('return-home-btn');
    const backToHomeSessionBtn = document.getElementById('back-to-home-session-btn');
    const nextBatchBtn = document.getElementById('next-batch-btn');
    const nextBatchHeaderBtn = document.getElementById('next-batch-header-btn');
    const prevBatchHeaderBtn = document.getElementById('prev-batch-header-btn');
    const completionText = completionMessage.querySelector('p');
    const backToDashboardBtn = document.getElementById('back-to-dashboard-btn');

    // Sidebar elements
    const statsTotal = document.getElementById('stats-total');
    const statsKnown = document.getElementById('stats-known');
    const statsReview = document.getElementById('stats-review');
    const reviewList = document.getElementById('review-list');
    
    // Dictionary elements
    const searchInput = document.getElementById('search-input');
    const searchBtn = document.getElementById('search-btn');
    const searchResult = document.getElementById('search-result');

    let words = [];
    let totalPages = 1;
    const ANIMATION_DELAY = 150; // ms, should be less than CSS transition time

    const initialAppState = {
        activeMode: null,
        sessions: {
            learn: { currentPage: 1, selections: {} },
            exam: { selections: {} },
            review: { currentPage: 1, selections: {} }
        }
    };
    let appState = JSON.parse(JSON.stringify(initialAppState));

    // ===== 用户认证函数 =====
    
    async function loadRecentUsers() {
        try {
            const response = await fetch('/users');
            const data = await response.json();
            
            if (data.success && data.users.length > 0) {
                recentUsersDiv.innerHTML = '';
                data.users.slice(0, 5).forEach(user => {
                    const userChip = document.createElement('div');
                    userChip.className = 'user-chip';
                    
                    const userInfo = document.createElement('div');
                    userInfo.className = 'user-chip-info';
                    userInfo.innerHTML = `
                        <span class="user-chip-name">${user.username}</span>
                        <span class="user-chip-time">${formatTime(user.last_login)}</span>
                    `;
                    userInfo.addEventListener('click', () => {
                        usernameInput.value = user.username;
                        handleLogin();
                    });
                    
                    const deleteBtn = document.createElement('button');
                    deleteBtn.className = 'user-delete-btn';
                    deleteBtn.innerHTML = '×';
                    deleteBtn.title = `Delete user ${user.username}`;
                    deleteBtn.addEventListener('click', (e) => {
                        e.stopPropagation(); // 防止触发用户选择
                        handleDeleteUser(user.username);
                    });
                    
                    userChip.appendChild(userInfo);
                    userChip.appendChild(deleteBtn);
                    recentUsersDiv.appendChild(userChip);
                });
            } else {
                recentUsersDiv.innerHTML = '<p style="color: #6b7280; font-size: 0.9rem;">No recent users</p>';
            }
        } catch (error) {
            console.error('Error loading recent users:', error);
        }
    }
    
    function formatTime(timestamp) {
        const date = new Date(timestamp * 1000);
        const now = new Date();
        const diffHours = Math.floor((now - date) / (1000 * 60 * 60));
        
        if (diffHours < 1) return 'Just now';
        if (diffHours < 24) return `${diffHours}h ago`;
        if (diffHours < 168) return `${Math.floor(diffHours / 24)}d ago`;
        return date.toLocaleDateString();
    }
    
    async function handleLogin() {
        const username = usernameInput.value.trim();
        
        if (!username) {
            showLoginMessage('Please enter a username', 'error');
            return;
        }
        
        if (username.length < 3 || username.length > 20) {
            showLoginMessage('Username must be 3-20 characters long', 'error');
            return;
        }
        
        if (!/^[a-zA-Z0-9_-]+$/.test(username)) {
            showLoginMessage('Username can only contain letters, numbers, underscore, and hyphen', 'error');
            return;
        }
        
        loginBtn.disabled = true;
        loginBtn.textContent = 'Logging in...';
        
        try {
            const response = await fetch('/login', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ username })
            });
            
            const data = await response.json();
            
            if (data.success) {
                currentUser = data.username;
                showLoginMessage(`Welcome ${data.is_new_user ? 'new user' : 'back'}, ${username}!`, 'success');
                
                setTimeout(() => {
                    loginContainer.classList.add('hidden');
                    appContainer.classList.remove('hidden');
                    updateUserInfo();
                    initializeDashboard();
                }, 1500);
            } else {
                showLoginMessage(data.error || 'Login failed', 'error');
            }
        } catch (error) {
            console.error('Login error:', error);
            showLoginMessage('Network error. Please try again.', 'error');
        } finally {
            loginBtn.disabled = false;
            loginBtn.textContent = 'Login / Register';
        }
    }
    
    function showLoginMessage(message, type) {
        loginMessage.textContent = message;
        loginMessage.className = `login-message ${type}`;
        loginMessage.style.display = 'block';
        
        if (type === 'success') {
            setTimeout(() => {
                loginMessage.style.display = 'none';
            }, 3000);
        }
    }
    
    async function updateUserInfo() {
        if (!currentUser) return;
        
        currentUsernameSpan.textContent = currentUser;
        
        try {
            const statsResponse = await fetch('/get_stats');
            const stats = await statsResponse.json();
            
            if (stats.success) {
                const completion = stats.stats.completion_percentage || 0;
                userProgressSpan.textContent = `Progress: ${completion.toFixed(1)}% (${stats.stats.current_position}/${stats.stats.total})`;
            }
        } catch (error) {
            console.error('Error updating user info:', error);
        }
    }
    
    async function handleLogout() {
        try {
            const response = await fetch('/logout', { method: 'POST' });
            const data = await response.json();
            
            if (data.success) {
                currentUser = null;
                appContainer.classList.add('hidden');
                loginContainer.classList.remove('hidden');
                usernameInput.value = '';
                loadRecentUsers();
            }
        } catch (error) {
            console.error('Logout error:', error);
        }
    }
    

    
    async function handleDeleteUser(username) {
        // 显示确认对话框
        const confirmed = confirm(`Are you sure you want to delete user "${username}"?\n\nThis action cannot be undone. All learning progress for this user will be permanently lost.`);
        
        if (!confirmed) {
            return;
        }
        
        try {
            const response = await fetch(`/users/${username}`, {
                method: 'DELETE'
            });
            
            const data = await response.json();
            
            if (data.success) {
                showLoginMessage(`User "${username}" has been deleted successfully.`, 'success');
                // 重新加载用户列表
                loadRecentUsers();
            } else {
                showLoginMessage(data.error || 'Failed to delete user', 'error');
            }
        } catch (error) {
            console.error('Error deleting user:', error);
            showLoginMessage('Network error. Please try again.', 'error');
        }
    }
    
    // 登录事件监听器
    loginBtn.addEventListener('click', handleLogin);
    logoutBtn.addEventListener('click', handleLogout);
    
    usernameInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') {
            handleLogin();
        }
    });
    
    // 初始化登录界面
    loadRecentUsers();

    function saveState() {
        localStorage.setItem('appState', JSON.stringify(appState));
    }

    function loadState() {
        const savedState = localStorage.getItem('appState');
        if (savedState) {
            appState = JSON.parse(savedState);
            return true;
        }
        return false;
    }

    function resetCurrentSession() {
        if (appState.activeMode) {
            appState.sessions[appState.activeMode] = JSON.parse(JSON.stringify(initialAppState.sessions[appState.activeMode]));
            saveState();
        }
    }
    
    function resetAllSessions() {
        appState = JSON.parse(JSON.stringify(initialAppState));
        localStorage.removeItem('appState');
    }

    async function initializeDashboard() {
        if (loadState() && appState.activeMode) {
            await startSession(appState.activeMode);
        } else {
            resetAllSessions();
            updateStats();
            updateReviewList();
            switchToView([welcomePanel, modeSelection]);
        }
    }

    async function updateStats() {
        try {
            const response = await fetch('/get_stats');
            const data = await response.json();
            
            if (data.success && data.stats) {
                statsTotal.textContent = data.stats.total;
                statsKnown.textContent = data.stats.known;
                statsReview.textContent = data.stats.review;
            } else {
                // 未登录或无数据时显示默认值
                statsTotal.textContent = '—';
                statsKnown.textContent = '—';
                statsReview.textContent = '—';
            }
        } catch (error) {
            console.error('Error fetching stats:', error);
            // 网络错误时显示默认值
            statsTotal.textContent = '—';
            statsKnown.textContent = '—';
            statsReview.textContent = '—';
        }
    }

    async function updateReviewList() {
        try {
            const response = await fetch('/get_all_review_words');
            const data = await response.json();
            reviewList.innerHTML = '';
            
            // 检查是否有有效的数据
            if (data.success && data.review_words && Array.isArray(data.review_words) && data.review_words.length > 0) {
                // 正确的API格式：data.review_words 数组
                data.review_words.forEach(item => {
                    const li = document.createElement('li');
                    
                    const wordText = document.createElement('span');
                    wordText.textContent = item.word;
                    wordText.className = 'review-word-text';
                    
                    const mistakeCount = document.createElement('span');
                    mistakeCount.textContent = item.mistakes;
                    mistakeCount.className = 'mistake-count';
                    
                    li.appendChild(wordText);
                    li.appendChild(mistakeCount);
                    reviewList.appendChild(li);
                });
            } else if (Array.isArray(data) && data.length > 0) {
                // API直接返回数组（旧格式兼容）
                data.forEach(item => {
                    const li = document.createElement('li');
                    
                    const wordText = document.createElement('span');
                    wordText.textContent = item.word;
                    wordText.className = 'review-word-text';
                    
                    const mistakeCount = document.createElement('span');
                    mistakeCount.textContent = item.mistakes;
                    mistakeCount.className = 'mistake-count';
                    
                    li.appendChild(wordText);
                    li.appendChild(mistakeCount);
                    reviewList.appendChild(li);
                });
            } else {
                // 没有复习单词或无数据
                const li = document.createElement('li');
                li.textContent = "No words to review yet!";
                li.classList.add('no-review-words');
                reviewList.appendChild(li);
            }
        } catch (error) {
            console.error('Error fetching review list:', error);
            // 发生错误时显示默认消息
            reviewList.innerHTML = '';
            const li = document.createElement('li');
            li.textContent = "No words to review yet!";
            li.classList.add('no-review-words');
            reviewList.appendChild(li);
        }
    }

    async function searchWord() {
        const word = searchInput.value.trim();
        if (!word) {
            showSearchResult('Please enter a word to search', 'placeholder');
            return;
        }

        // Validate input
        if (word.length > 50) {
            showSearchResult('Word too long. Please enter a shorter word.', 'error');
            return;
        }

        if (!/^[a-zA-Z\s\-']+$/.test(word)) {
            showSearchResult('Please enter a valid English word (letters only).', 'error');
            return;
        }

        // Show loading state
        showSearchResult('🔍 Searching in dictionary...', 'loading');
        searchBtn.disabled = true;
        searchBtn.textContent = 'Searching...';
        
        try {
            const controller = new AbortController();
            const timeoutId = setTimeout(() => {
                controller.abort();
                showSearchResult('Search timeout. Please try again.', 'error');
                searchBtn.disabled = false;
                searchBtn.textContent = 'Search';
            }, 15000); // 15秒超时
            
            const response = await fetch(`/dictionary_search?word=${encodeURIComponent(word)}`, {
                signal: controller.signal
            });
            
            clearTimeout(timeoutId);
            
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            const result = await response.json();
            
            if (result.success) {
                displayDefinition(result.data);
            } else {
                const errorMsg = result.error || 'Search failed';
                if (errorMsg.includes('Word not found')) {
                    showSearchResult(`Sorry, "${word}" was not found in the dictionary. Please check the spelling or try a different word.`, 'not-found');
                } else if (errorMsg.includes('Network') || errorMsg.includes('API')) {
                    showSearchResult('Dictionary service is temporarily unavailable. Please try again later.', 'error');
                } else {
                    showSearchResult(errorMsg, 'error');
                }
            }
        } catch (error) {
            console.error('Dictionary search error:', error);
            
            if (error.name === 'AbortError') {
                showSearchResult('Search timeout. Please try again.', 'error');
            } else if (error.message.includes('Failed to fetch')) {
                showSearchResult('Network connection error. Please check your internet connection.', 'error');
            } else {
                showSearchResult('An unexpected error occurred. Please try again.', 'error');
            }
        } finally {
            searchBtn.disabled = false;
            searchBtn.textContent = 'Search';
        }
    }

    function showSearchResult(message, type = 'placeholder') {
        searchResult.innerHTML = `<p class="search-${type}">${message}</p>`;
    }



    function displayDefinition(data) {
        const { word, phonetics, meanings, chinese_translation, source } = data;
        
        let html = `<div class="word-definition">`;
        
        // 单词标题
        html += `<div class="word-title">
            <span class="word-text">${word}</span>
            <span class="word-actions">
                <button class="copy-btn" onclick="navigator.clipboard.writeText('${word}')" title="Copy word">📋</button>
            </span>
        </div>`;
        
        // 中文翻译（如果可用，优先显示）
        if (chinese_translation && chinese_translation !== "未找到详细释义，建议使用其他词典查询") {
            html += `<div class="chinese-translation">
                <span class="translation-icon">🇨🇳</span>
                <span class="translation-text">${chinese_translation}</span>
            </div>`;
        }
        
        // 音标信息
        if (phonetics && phonetics.length > 0) {
            html += `<div class="pronunciation-section">`;
            phonetics.forEach(p => {
                if (p.text) {
                    html += `<div class="pronunciation">
                        <span class="phonetic-text">${p.text}</span>
                        ${p.audio ? `<button class="audio-btn" onclick="playAudio('${p.audio}')" title="Play pronunciation">🔊</button>` : ''}
                    </div>`;
                }
            });
            html += `</div>`;
        }
        
        // 词义信息
        if (meanings && meanings.length > 0) {
            html += `<div class="meanings-section">`;
            meanings.forEach((meaning, index) => {
                html += `<div class="meaning-group">`;
                
                if (meaning.partOfSpeech) {
                    html += `<div class="part-of-speech">${meaning.partOfSpeech}</div>`;
                }
                
                if (meaning.definitions && meaning.definitions.length > 0) {
                    html += `<ol class="definitions-list">`;
                    meaning.definitions.forEach((def, defIndex) => {
                        html += `<li class="definition-item">`;
                        
                        if (def.definition) {
                            html += `<div class="definition">${def.definition}</div>`;
                        }
                        
                        if (def.example) {
                            html += `<div class="example">
                                <span class="example-icon">💬</span>
                                <em>"${def.example}"</em>
                            </div>`;
                        }
                        
                        if (def.synonyms && def.synonyms.length > 0) {
                            html += `<div class="synonyms">
                                <span class="synonyms-label">Synonyms:</span>
                                <span class="synonyms-list">${def.synonyms.join(', ')}</span>
                            </div>`;
                        }
                        
                        html += `</li>`;
                    });
                    html += `</ol>`;
                }
                
                html += `</div>`;
            });
            html += `</div>`;
        }
        
        // 来源信息
        if (source) {
            html += `<div class="source">
                <span class="source-icon">📚</span>
                <span class="source-text">Source: ${source}</span>
            </div>`;
        }
        
        html += `</div>`;
        searchResult.innerHTML = html;
    }

    // 音频播放功能
    window.playAudio = function(audioUrl) {
        if (audioUrl && audioUrl !== '') {
            const audio = new Audio(audioUrl);
            audio.play().catch(e => {
                console.warn('Audio playback failed:', e);
                showSearchResult('Audio playback not available', 'warning');
            });
        }
    };

    function switchToView(viewsToShow) {
        if (!Array.isArray(viewsToShow)) {
            viewsToShow = [viewsToShow];
        }

        const allViews = [modeSelection, wordDisplay, completionMessage, welcomePanel, listenInterface];
        
        allViews.forEach(view => view.classList.add('fading'));

        setTimeout(() => {
            allViews.forEach(view => view.classList.add('hidden'));
            
            viewsToShow.forEach(view => {
                if (view) {
                    view.classList.remove('hidden');
                    setTimeout(() => view.classList.remove('fading'), 20);
                }
            });
            
            // 控制批次按钮的显示
            updateBatchButtonsVisibility();
        }, ANIMATION_DELAY);
    }
    
    function updateBatchButtonsVisibility() {
        const isInSessionMode = (appState.activeMode === 'learn' || appState.activeMode === 'review') && !wordDisplay.classList.contains('hidden');
        const currentPage = appState.sessions[appState.activeMode]?.currentPage || 1;
        const hasNextPage = isInSessionMode && currentPage < totalPages;
        const hasPrevPage = isInSessionMode && currentPage > 1;
        
        if (isInSessionMode) {
            if (hasNextPage) {
                nextBatchHeaderBtn.classList.remove('hidden');
                nextBatchHeaderBtn.textContent = appState.activeMode === 'review' ? '下一页' : '下一批';
            } else {
                nextBatchHeaderBtn.classList.add('hidden');
            }
            
            if (hasPrevPage) {
                prevBatchHeaderBtn.classList.remove('hidden');
                prevBatchHeaderBtn.textContent = appState.activeMode === 'review' ? '上一页' : '上一批';
            } else {
                prevBatchHeaderBtn.classList.add('hidden');
            }
        } else {
            nextBatchHeaderBtn.classList.add('hidden');
            prevBatchHeaderBtn.classList.add('hidden');
        }
    }

    async function startSession(mode) {
        appState.activeMode = mode;
        const currentSession = appState.sessions[mode];
        let url = '';

        if (mode === 'learn') {
            url = `/get_learn_words?page=${currentSession.currentPage}`;
        } else if (mode === 'exam') {
            url = '/get_exam_words';
        } else if (mode === 'review') {
            url = `/get_review_words?page=${currentSession.currentPage}`;
        }

        try {
            const response = await fetch(url);
            const data = await response.json();
            
            if (mode === 'learn') {
                words = data.words;
                totalPages = data.totalPages;
            } else if (mode === 'review') {
                // Review mode now supports pagination like learn mode
                if (data.success && data.review_words) {
                    words = data.review_words.map(item => item.word);
                    totalPages = data.totalPages || 1;
                } else {
                    words = [];
                    totalPages = 1;
                }
            } else {
                // For exam mode, check if it's the new format with success field
                if (data.success && data.words) {
                    words = data.words;
                } else if (Array.isArray(data)) {
                    // Legacy format: direct array
                    words = data.map(item => item.word || item);
                } else {
                    words = [];
                }
            }

            if (words.length === 0) {
                let message;
                if (mode === 'review') {
                    message = data.completed ? data.message || 'Congratulations! You have reviewed all mistake words.' : 'No words to review. Great job!';
                } else if (mode === 'learn') {
                    message = data.completed ? data.message || 'Congratulations! You have completed all words.' : 'You have learned all the words!';
                } else {
                    message = 'No words found.';
                }
                showCompletion(message);
                resetCurrentSession();
                return;
            }
            
            saveState();
            displayWordList();
            switchToView(wordDisplay);
            updateStats();
            updateReviewList();
            updateBatchButtonsVisibility();
        } catch (error) {
            console.error('Error fetching words:', error);
            alert('Failed to load words. Please try again.');
        }
    }

    function displayWordList() {
        wordList.innerHTML = '';
        const currentSelections = appState.sessions[appState.activeMode].selections;

        words.forEach(word => {
            const li = document.createElement('li');
            li.className = 'word-item';
            li.dataset.word = word;

            const wordText = document.createElement('span');
            wordText.className = 'word-text';
            wordText.textContent = word;

            const actions = document.createElement('div');
            actions.className = 'word-actions';

            const knowBtn = document.createElement('button');
            knowBtn.textContent = 'Know';
            knowBtn.className = 'know';

            const dontKnowBtn = document.createElement('button');
            dontKnowBtn.textContent = 'Don\'t Know';
            dontKnowBtn.className = 'dont-know';
            
            actions.appendChild(knowBtn);
            actions.appendChild(dontKnowBtn);
            li.appendChild(wordText);
            li.appendChild(actions);

            if (currentSelections[word]) {
                const selection = currentSelections[word];
                if (selection === 'know') {
                    knowBtn.classList.add('selected');
                } else if (selection === 'dont-know') {
                    dontKnowBtn.classList.add('selected');
                }
            }

            knowBtn.addEventListener('click', () => handleSelection(word, 'know', knowBtn, dontKnowBtn));
            dontKnowBtn.addEventListener('click', () => handleSelection(word, 'dont-know', dontKnowBtn, knowBtn));

            wordList.appendChild(li);
        });
        
        submitBtn.classList.add('hidden');
        checkAllWordsSelected();
    }

    function handleSelection(word, selection, selectedBtn, otherBtn) {
        appState.sessions[appState.activeMode].selections[word] = selection;
        saveState();
        
        selectedBtn.classList.add('selected');
        otherBtn.classList.remove('selected');

        checkAllWordsSelected();
    }

    function checkAllWordsSelected() {
        const currentSession = appState.sessions[appState.activeMode];
        if (Object.keys(currentSession.selections).length === words.length) {
            submitBtn.classList.remove('hidden');
        }
    }

    async function submitResults() {
        const currentSession = appState.sessions[appState.activeMode];
        const wordsToUpdate = Object.entries(currentSession.selections)
            .filter(([, selection]) => selection === 'dont-know')
            .map(([word]) => word);

        // Update mistake counts for all modes (including exam)
        if (wordsToUpdate.length > 0) {
            try {
                await fetch('/update_mistakes_batch', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ words: wordsToUpdate }),
                });
                // Update the review list and stats after updating mistakes
                await updateReviewList();
                await updateStats();
            } catch (error) {
                console.error('Error updating mistakes:', error);
                alert('Failed to submit results. Please try again.');
                return;
            }
        }

        if (appState.activeMode === 'learn') {
            if (currentSession.currentPage < totalPages) {
                currentSession.currentPage++;
                currentSession.selections = {};
                saveState();
                startSession('learn');
            } else {
                showCompletion('Congratulations! You have learned all the words.');
                resetCurrentSession();
            }
        } else if (appState.activeMode === 'review') {
            if (currentSession.currentPage < totalPages) {
                currentSession.currentPage++;
                currentSession.selections = {};
                saveState();
                startSession('review');
            } else {
                showCompletion('Congratulations! You have reviewed all mistake words.');
                resetCurrentSession();
            }
        } else if (appState.activeMode === 'exam') {
            const correctAnswers = Object.values(currentSession.selections).filter(s => s === 'know').length;
            const incorrectAnswers = Object.values(currentSession.selections).filter(s => s === 'dont-know').length;
            let scoreMessage = `Exam finished! <br> Your score: <span class="score">${correctAnswers} / ${words.length}</span>`;
            if (incorrectAnswers > 0) {
                scoreMessage += `<br><span class="mistakes-added">Added ${incorrectAnswers} words to review list</span>`;
            }
            showCompletion(scoreMessage);
            resetCurrentSession();
        } else {
            showCompletion('Session complete! Your results have been saved.');
            resetCurrentSession();
        }
    }

    function showCompletion(message) {
        switchToView(completionMessage);
        nextBatchBtn.classList.add('hidden');
        completionText.innerHTML = message;
    }
    
    function resetToHome() {
        // Stop any ongoing reading if in listen mode
        if (appState.activeMode === 'listen') {
            window.stopReading();
        }
        
        appState.activeMode = null;
        saveState();
        updateStats();
        updateReviewList();
        switchToView([welcomePanel, modeSelection]);
    }
    
    async function goToNextBatch() {
        if (appState.activeMode !== 'learn' && appState.activeMode !== 'review') {
            return;
        }
        
        const currentSession = appState.sessions[appState.activeMode];
        if (currentSession.currentPage < totalPages) {
            currentSession.currentPage++;
            currentSession.selections = {};
            saveState();
            await startSession(appState.activeMode);
        }
    }

    async function goToPrevBatch() {
        if (appState.activeMode !== 'learn' && appState.activeMode !== 'review') {
            return;
        }
        
        const currentSession = appState.sessions[appState.activeMode];
        if (currentSession.currentPage > 1) {
            currentSession.currentPage--;
            currentSession.selections = {};
            saveState();
            await startSession(appState.activeMode);
        }
    }

    // This function is no longer needed with the new model
    // function learnNextBatch() { ... }

    learnModeBtn.addEventListener('click', () => startSession('learn'));
    examModeBtn.addEventListener('click', () => startSession('exam'));
    reviewModeBtn.addEventListener('click', () => startSession('review'));
    wordReaderBtn.addEventListener('click', () => {
        appState.activeMode = 'listen';
        saveState();
        switchToView(listenInterface);
        initializeListenInterface();
    });
    submitBtn.addEventListener('click', submitResults);
    returnHomeBtn.addEventListener('click', resetToHome);
    backToHomeSessionBtn.addEventListener('click', resetToHome);
    nextBatchHeaderBtn.addEventListener('click', goToNextBatch);
    prevBatchHeaderBtn.addEventListener('click', goToPrevBatch);
    // nextBatchBtn is part of the completion screen, which now has its own logic
    // nextBatchBtn.addEventListener('click', learnNextBatch);

    // Dictionary search event listeners
    searchBtn.addEventListener('click', searchWord);
    searchInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') {
            searchWord();
        }
    });
    searchInput.addEventListener('input', () => {
        if (!searchInput.value.trim()) {
            showSearchResult('Enter a word to see its meaning', 'placeholder');
        }
    });

    // Listen interface event listeners
    backToDashboardBtn.addEventListener('click', () => {
        resetToHome();
    });

    // ===== 单词朗读器功能 =====
    let listenWords = [];
    let currentWordIndex = 0;
    let isReading = false;
    let isPaused = false;
    let readingTimeout;

    function initializeListenInterface() {
        // Reset listen interface state
        listenWords = [];
        currentWordIndex = 0;
        isReading = false;
        isPaused = false;
        clearTimeout(readingTimeout);
        
        // Reset UI elements
        const wordTextArea = document.getElementById('wordText');
        const speedSlider = document.getElementById('speed');
        const pauseSlider = document.getElementById('pause');
        const speedValue = document.getElementById('speedValue');
        const pauseValue = document.getElementById('pauseValue');
        const startBtn = document.getElementById('startBtn');
        const pauseBtn = document.getElementById('pauseBtn');
        const stopBtn = document.getElementById('stopBtn');
        const progressFill = document.getElementById('progressFill');
        const status = document.getElementById('status');
        const wordListDiv = document.getElementById('wordList');
        const wordListTip = document.getElementById('wordListTip');

        // Reset form values
        if (wordTextArea) wordTextArea.value = '';
        if (speedSlider) speedSlider.value = '150';
        if (pauseSlider) pauseSlider.value = '800';
        if (speedValue) speedValue.textContent = '150';
        if (pauseValue) pauseValue.textContent = '800';
        
        // Reset button states
        if (startBtn) startBtn.disabled = false;
        if (pauseBtn) {
            pauseBtn.disabled = true;
            pauseBtn.textContent = '暂停';
        }
        if (stopBtn) stopBtn.disabled = true;
        
        // Reset progress and status
        if (progressFill) progressFill.style.width = '0%';
        if (status) status.style.display = 'none';
        if (wordListDiv) wordListDiv.style.display = 'none';
        if (wordListTip) wordListTip.style.display = 'none';

        // Setup event listeners for listen interface
        setupListenEventListeners();
        
        // Initialize speech synthesis
        initSpeechSynthesis();
    }

    function setupListenEventListeners() {
        const speedSlider = document.getElementById('speed');
        const pauseSlider = document.getElementById('pause');
        const speedValue = document.getElementById('speedValue');
        const pauseValue = document.getElementById('pauseValue');

        if (speedSlider && speedValue) {
            speedSlider.addEventListener('input', function() {
                speedValue.textContent = this.value;
            });
        }

        if (pauseSlider && pauseValue) {
            pauseSlider.addEventListener('input', function() {
                pauseValue.textContent = this.value;
            });
        }

        // Keyboard shortcuts
        document.addEventListener('keydown', function(e) {
            if (listenInterface && !listenInterface.classList.contains('hidden')) {
                if (e.ctrlKey) {
                    switch(e.key) {
                        case 'Enter':
                            e.preventDefault();
                            if (!isReading) {
                                startReading();
                            }
                            break;
                        case ' ':
                            e.preventDefault();
                            if (isReading) {
                                pauseReading();
                            }
                            break;
                        case 'Escape':
                            e.preventDefault();
                            stopReading();
                            break;
                    }
                }
            }
        });
    }

    window.startReading = function() {
        const wordTextArea = document.getElementById('wordText');
        const text = wordTextArea ? wordTextArea.value.trim() : '';
        
        if (!text) {
            showListenStatus('请先输入要朗读的单词！', 'error');
            return;
        }

        listenWords = parseWords(text);
        if (listenWords.length === 0) {
            showListenStatus('没有找到有效的单词！', 'error');
            return;
        }

        currentWordIndex = 0;
        isReading = true;
        isPaused = false;

        // Update button states
        const startBtn = document.getElementById('startBtn');
        const pauseBtn = document.getElementById('pauseBtn');
        const stopBtn = document.getElementById('stopBtn');
        
        if (startBtn) startBtn.disabled = true;
        if (pauseBtn) pauseBtn.disabled = false;
        if (stopBtn) stopBtn.disabled = false;

        showListenStatus(`准备朗读 ${listenWords.length} 个单词...`, 'info');
        displayListenWords();
        
        readNextWord();
    };

    window.pauseReading = function() {
        if (isReading) {
            isPaused = !isPaused;
            const pauseBtn = document.getElementById('pauseBtn');
            
            if (isPaused) {
                clearTimeout(readingTimeout);
                if (speechSynthesis) speechSynthesis.pause();
                if (pauseBtn) pauseBtn.textContent = '继续';
                showListenStatus('朗读已暂停', 'info');
            } else {
                if (speechSynthesis) speechSynthesis.resume();
                if (pauseBtn) pauseBtn.textContent = '暂停';
                readNextWord();
            }
        }
    };

    window.stopReading = function() {
        isReading = false;
        isPaused = false;
        clearTimeout(readingTimeout);
        if (speechSynthesis) speechSynthesis.cancel();

        // Reset button states
        const startBtn = document.getElementById('startBtn');
        const pauseBtn = document.getElementById('pauseBtn');
        const stopBtn = document.getElementById('stopBtn');
        
        if (startBtn) startBtn.disabled = false;
        if (pauseBtn) {
            pauseBtn.disabled = true;
            pauseBtn.textContent = '暂停';
        }
        if (stopBtn) stopBtn.disabled = true;

        currentWordIndex = 0;
        updateListenProgress();
        displayListenWords();
        hideListenStatus();
    };

    window.clearText = function() {
        const wordTextArea = document.getElementById('wordText');
        if (wordTextArea) wordTextArea.value = '';
        
        window.stopReading();
        listenWords = [];
        
        const wordListDiv = document.getElementById('wordList');
        const wordListTip = document.getElementById('wordListTip');
        if (wordListDiv) wordListDiv.style.display = 'none';
        if (wordListTip) wordListTip.style.display = 'none';
        
        hideListenStatus();
    };

    window.testSpeech = async function() {
        showListenStatus('测试语音功能...', 'info');
        console.log('开始测试语音功能');
        
        try {
            await speakWord('hello');
            showListenStatus('语音测试成功！', 'success');
        } catch (error) {
            console.error('语音测试失败:', error);
            showListenStatus('语音测试失败，请检查浏览器设置', 'error');
        }
    };

    function parseWords(text) {
        return text.toLowerCase()
                  .split(/\s+/)
                  .map(word => word.replace(/[^\w]/g, ''))
                  .filter(word => word.length > 0);
    }

    function showListenStatus(message, type = 'info') {
        const status = document.getElementById('status');
        if (status) {
            status.innerHTML = `${message}<button class="close-btn" onclick="hideListenStatus()" title="关闭">×</button>`;
            status.className = `status ${type}`;
            status.style.display = 'block';
        }
    }

    window.hideListenStatus = function() {
        const status = document.getElementById('status');
        if (status) status.style.display = 'none';
    };

    function updateListenProgress() {
        const progress = listenWords.length > 0 ? (currentWordIndex / listenWords.length) * 100 : 0;
        const progressFill = document.getElementById('progressFill');
        if (progressFill) progressFill.style.width = progress + '%';
    }

    function displayListenWords() {
        const wordListDiv = document.getElementById('wordList');
        const wordListTip = document.getElementById('wordListTip');
        
        if (!wordListDiv || !wordListTip) return;
        
        if (listenWords.length === 0) {
            wordListDiv.style.display = 'none';
            wordListTip.style.display = 'none';
            return;
        }

        wordListDiv.innerHTML = '';
        listenWords.forEach((word, index) => {
            const wordElement = document.createElement('span');
            wordElement.className = 'word-item';
            if (index === currentWordIndex) {
                wordElement.classList.add('current');
            }
            wordElement.textContent = word;
            
            // Add click event for single word reading
            wordElement.onclick = () => {
                speakSingleWord(word, index);
            };
            
            wordElement.style.cursor = 'pointer';
            wordElement.title = `点击朗读: ${word}`;
            
            wordListDiv.appendChild(wordElement);
        });
        
        wordListDiv.style.display = 'block';
        wordListTip.style.display = 'block';
    }

    async function speakSingleWord(word, index) {
        // Temporarily highlight the clicked word
        const wordElements = document.querySelectorAll('.word-item');
        wordElements.forEach((el, i) => {
            el.classList.remove('clicked');
            if (i === index) {
                el.classList.add('clicked');
            }
        });
        
        showListenStatus(`点击朗读: ${word}`, 'info');
        
        try {
            await speakWord(word);
            showListenStatus(`朗读完成: ${word}`, 'success');
            
            setTimeout(() => {
                wordElements.forEach(el => el.classList.remove('clicked'));
            }, 1000);
        } catch (error) {
            showListenStatus(`朗读失败: ${word}`, 'error');
            setTimeout(() => {
                wordElements.forEach(el => el.classList.remove('clicked'));
            }, 1000);
        }
    }

    function speakWord(word) {
        return new Promise((resolve) => {
            console.log('尝试朗读单词:', word);
            
            // Use Web Speech API
            if ('speechSynthesis' in window) {
                console.log('使用Web Speech API');
                
                if (speechSynthesis.speaking) {
                    speechSynthesis.cancel();
                }
                
                const utterance = new SpeechSynthesisUtterance(word);
                const speedSlider = document.getElementById('speed');
                utterance.rate = speedSlider ? (speedSlider.value / 150) : 1;
                utterance.lang = 'en-US';
                utterance.volume = 1;
                utterance.pitch = 1;
                
                utterance.onstart = () => {
                    console.log('开始朗读:', word);
                };
                
                utterance.onend = () => {
                    console.log('朗读完成:', word);
                    resolve();
                };
                
                utterance.onerror = (event) => {
                    console.error('Speech synthesis error for word:', word, event);
                    showListenStatus(`朗读错误: ${event.error}`, 'error');
                    resolve();
                };
                
                // Get available voices
                const voices = speechSynthesis.getVoices();
                if (voices.length > 0) {
                    const englishVoice = voices.find(voice => voice.lang.startsWith('en'));
                    if (englishVoice) {
                        utterance.voice = englishVoice;
                        console.log('使用语音:', englishVoice.name);
                    }
                }
                
                setTimeout(() => {
                    speechSynthesis.speak(utterance);
                }, 100);
                
            } else {
                console.log('Web Speech API不支持，使用后端');
                // Fallback to backend if Web Speech API is not supported
                fetch('/speak', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({ word: word })
                }).then(response => {
                    if (response.ok) {
                        console.log('后端朗读成功:', word);
                    } else {
                        console.error('后端朗读失败:', response.status);
                    }
                    setTimeout(resolve, 1000);
                }).catch(error => {
                    console.error('Backend speech error for word:', word, error);
                    showListenStatus(`后端朗读错误: ${error.message}`, 'error');
                    setTimeout(resolve, 500);
                });
            }
        });
    }

    async function readNextWord() {
        if (!isReading || isPaused || currentWordIndex >= listenWords.length) {
            return;
        }

        const word = listenWords[currentWordIndex];
        showListenStatus(`正在朗读: ${word} (${currentWordIndex + 1}/${listenWords.length})`, 'info');
        
        displayListenWords();
        updateListenProgress();

        await speakWord(word);
        
        if (!isReading || isPaused) return;
        
        currentWordIndex++;
        
        if (currentWordIndex >= listenWords.length) {
            // Reading complete
            window.stopReading();
            showListenStatus('朗读完成！', 'success');
            return;
        }

        // Pause between words
        const pauseSlider = document.getElementById('pause');
        const pauseTime = pauseSlider ? parseInt(pauseSlider.value) : 800;
        readingTimeout = setTimeout(readNextWord, pauseTime);
    }

    function initSpeechSynthesis() {
        if ('speechSynthesis' in window) {
            const loadVoices = () => {
                const voices = speechSynthesis.getVoices();
                console.log('可用语音数量:', voices.length);
                voices.forEach(voice => {
                    console.log(`语音: ${voice.name} (${voice.lang})`);
                });
                
                if (voices.length > 0) {
                    showListenStatus(`语音合成已准备就绪，共${voices.length}个语音可用`, 'success');
                    setTimeout(() => hideListenStatus(), 3000);
                }
            };
            
            if (speechSynthesis.getVoices().length > 0) {
                loadVoices();
            } else {
                speechSynthesis.onvoiceschanged = loadVoices;
            }
        } else {
            showListenStatus('您的浏览器不支持语音合成，将尝试使用后端服务', 'info');
        }
    }

    initializeDashboard();
});

