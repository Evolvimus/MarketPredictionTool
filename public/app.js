// DOM Elements - Navigation
const navItems = document.querySelectorAll('.nav-item');
const views = document.querySelectorAll('.view-route');
const viewTitle = document.getElementById('viewTitle');
const viewSubtitle = document.getElementById('viewSubtitle');
const currentTimeEl = document.getElementById('currentTime');

// DOM Elements - Analysis
const analyzeBtn = document.getElementById('analyzeBtn');
const tickerInput = document.getElementById('ticker');
const modelSelect = document.getElementById('model');
const resultsSection = document.getElementById('results');
const loadingOverlay = document.getElementById('loading');
const autocompleteDropdown = document.getElementById('autocompleteDropdown');

// DOM Elements - Indicators
const entryPriceEl = document.getElementById('entryPrice');
const takeProfitEl = document.getElementById('takeProfit');
const stopLossEl = document.getElementById('stopLoss');
const rsiDisplayEl = document.getElementById('rsiDisplay');
const predictionEl = document.getElementById('prediction');
const chartContainer = document.getElementById('chart');

// DOM Elements - Dashboard
const totalAnalysesEl = document.getElementById('totalAnalyses');
const winRateEl = document.getElementById('winRate');
const totalREl = document.getElementById('totalR');
const vetoRateEl = document.getElementById('vetoRate');
const activityFeedEl = document.getElementById('activityFeed');

// Global state
let currentAnalysisId = null;
let selectedIndex = -1;
let filteredSymbols = [];
let chart = null;
let candlestickSeries = null;
let modalCallback = null;

const symbolDatabase = [
    { symbol: 'AAPL', name: 'Apple Inc.', category: 'stock' },
    { symbol: 'MSFT', name: 'Microsoft Corporation', category: 'stock' },
    { symbol: 'TSLA', name: 'Tesla Inc.', category: 'stock' },
    { symbol: 'BTC-USD', name: 'Bitcoin USD', category: 'crypto' },
    { symbol: 'ETH-USD', name: 'Ethereum USD', category: 'crypto' },
    { symbol: 'GC=F', name: 'Gold (XAUUSD)', category: 'commodity' },
    { symbol: 'SI=F', name: 'Silver', category: 'commodity' },
    { symbol: 'PL=F', name: 'Platinum', category: 'commodity' },
    { symbol: 'EURUSD=X', name: 'EUR / USD', category: 'forex' },
    { symbol: 'LMT', name: 'Lockheed Martin', category: 'stock' },
    { symbol: 'RHM.DE', name: 'Rheinmetall AG', category: 'stock' },
    { symbol: 'RTX', name: 'RTX Corporation', category: 'stock' }
];

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    initClock();
    initNavigation();
    initAutocomplete();
    initQuickSymbols();
    initModal();
    initSettings();
    loadDashboardStats();

    // Default view: Dashboard
    switchView('dashboard');
});

async function initSettings() {
    try {
        const res = await fetch('/api/settings');
        const settings = await res.json();
        document.getElementById('accountBalance').value = settings.account_balance;
        document.getElementById('riskPerTrade').value = settings.risk_per_trade_pct;
    } catch (e) {
        console.error('Settings load failed', e);
    }
}

window.saveUserSettings = async () => {
    const account_balance = parseFloat(document.getElementById('accountBalance').value);
    const risk_per_trade_pct = parseFloat(document.getElementById('riskPerTrade').value);

    try {
        const res = await fetch('/api/settings', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ account_balance, risk_per_trade_pct })
        });
        if (res.ok) {
            alert('Einstellungen erfolgreich gespeichert!');
        }
    } catch (e) {
        alert('Speichern fehlgeschlagen: ' + e.message);
    }
};

function initModal() {
    const cancelBtn = document.getElementById('modalCancel');
    const confirmBtn = document.getElementById('modalConfirm');
    const modal = document.getElementById('customModal');

    cancelBtn.addEventListener('click', () => {
        modal.style.display = 'none';
        if (modalCallback) modalCallback(false);
    });

    confirmBtn.addEventListener('click', () => {
        modal.style.display = 'none';
        if (modalCallback) modalCallback(true);
    });
}

function showCustomModal(title, message, confirmText = 'Bestätigen') {
    return new Promise((resolve) => {
        const modal = document.getElementById('customModal');
        const titleEl = document.getElementById('modalTitle');
        const msgEl = document.getElementById('modalMessage');
        const confirmBtn = document.getElementById('modalConfirm');

        titleEl.textContent = title;
        msgEl.textContent = message;
        confirmBtn.textContent = confirmText;
        modal.style.display = 'flex';

        modalCallback = resolve;
    });
}

// Clock
function initClock() {
    const update = () => {
        const now = new Date();
        currentTimeEl.textContent = now.toLocaleTimeString('de-DE', { hour: '2-digit', minute: '2-digit' });
    };
    update();
    setInterval(update, 1000 * 60);
}

// Navigation
function initNavigation() {
    navItems.forEach(item => {
        item.addEventListener('click', () => {
            const target = item.dataset.view;
            if (target) switchView(target);
        });
    });
}

function switchView(viewId) {
    // Update Nav UI
    navItems.forEach(item => {
        item.classList.toggle('active', item.dataset.view === viewId);
    });

    // Update View UI
    views.forEach(view => {
        view.classList.toggle('active', view.id === `${viewId}View`);
    });

    // Update Header
    const titles = {
        dashboard: { main: 'Dashboard', sub: 'Ihre Handelsübersicht auf einen Blick.' },
        analyze: { main: 'Marktanalyse', sub: 'KI-gestützte Taktik und Prognose.' },
        history: { main: 'Historie', sub: 'Vergangene Analysen und Feedback.' },
        settings: { main: 'Einstellungen', sub: 'Modell-Konfiguration und API.' }
    };

    if (titles[viewId]) {
        viewTitle.textContent = titles[viewId].main;
        viewSubtitle.textContent = titles[viewId].sub;
    }

    if (viewId === 'history') loadRecentAnalyses();
    if (viewId === 'dashboard') loadDashboardStats();
}

// Dashboard Logic
async function loadDashboardStats() {
    try {
        const response = await fetch('/api/recent-analyses');
        if (!response.ok) throw new Error('API Error');
        const analyses = await response.json();

        // 1. Total Analyses
        totalAnalysesEl.textContent = analyses.length;

        // 2. Win Rate & Total R
        const feedbacks = analyses.filter(a => a.feedback && a.feedback.submitted);
        const wins = feedbacks.filter(a => a.feedback.success).length;
        const winPct = feedbacks.length > 0 ? (wins / feedbacks.length * 100).toFixed(0) : 0;
        winRateEl.textContent = `${winPct}%`;

        // 3. Veto Rate
        const vetoes = analyses.filter(a => a.ai_prediction && a.ai_prediction.decision === 'veto').length;
        const vetoPct = analyses.length > 0 ? (vetoes / analyses.length * 100).toFixed(0) : 0;
        vetoRateEl.textContent = `${vetoPct}%`;

        // Net R (Mock calculation for UI)
        let totalR = feedbacks.reduce((acc, curr) => {
            return acc + (curr.feedback.success ? (curr.trading_levels.take_profit / curr.trading_levels.entry - 1) * 10 : -1);
        }, 0);
        totalREl.textContent = (totalR * 0.5).toFixed(1); // Scaled for demo

        // Populate Activity Feed (Top 5)
        renderActivityFeed(analyses.slice(0, 5));

    } catch (e) {
        console.error('Stats failed', e);
    }
}

function renderActivityFeed(analyses) {
    activityFeedEl.innerHTML = '';
    if (analyses.length === 0) {
        activityFeedEl.innerHTML = '<p class="text-dim" style="text-align:center; padding: 2rem;">Keine Aktivitäten vorhanden.</p>';
        return;
    }

    analyses.forEach(a => {
        const item = document.createElement('div');
        item.className = 'activity-item';
        const isSuccess = a.feedback?.submitted ? (a.feedback.success ? 'success' : 'danger') : 'dim';

        item.innerHTML = `
            <div style="display:flex; align-items:center; gap: 1rem;">
                <div class="widget-icon" style="margin:0; width:36px; height:36px; font-size: 0.9rem;">
                    ${a.ticker.includes('-') ? '₿' : '📈'}
                </div>
                <div>
                    <div style="font-weight:600;">${a.ticker} Analyse</div>
                    <div style="font-size: 0.75rem; color: var(--text-dim);">${a.timestamp}</div>
                </div>
            </div>
            <div style="text-align:right;">
                <div style="font-weight:700; color: var(--${a.feedback?.submitted ? (a.feedback.success ? 'success' : 'danger') : 'text-primary'});">
                    ${a.feedback?.submitted ? (a.feedback.success ? '+ Profit' : '- Loss') : 'Anstehend'}
                </div>
                <div style="font-size: 0.75rem; color: var(--text-dim);">$${a.trading_levels.entry.toFixed(1)}</div>
            </div>
        `;
        activityFeedEl.appendChild(item);
    });
}

// Chart Logic
function initChart() {
    chartContainer.innerHTML = '';
    chart = LightweightCharts.createChart(chartContainer, {
        layout: { background: { color: 'transparent' }, textColor: '#94a3b8' },
        grid: { vertLines: { color: 'rgba(255, 255, 255, 0.05)' }, horzLines: { color: 'rgba(255, 255, 255, 0.05)' } },
        width: chartContainer.clientWidth,
        height: 400,
    });
    candlestickSeries = chart.addCandlestickSeries({
        upColor: '#10b981', downColor: '#ef4444',
        borderUpColor: '#10b981', borderDownColor: '#ef4444',
        wickUpColor: '#10b981', wickDownColor: '#ef4444',
    });
}

// Analysis API
async function startAnalysis() {
    const ticker = tickerInput.value.trim().toUpperCase();
    const model = modelSelect.value;
    if (!ticker) return;

    loadingOverlay.style.display = 'flex';
    resultsSection.style.display = 'none';

    try {
        const res = await fetch('/api/analyze', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ ticker, model }),
        });

        if (!res.ok) throw new Error(await res.text());
        const data = await res.json();

        // Update UI
        entryPriceEl.textContent = `$${data.trading_levels.entry.toFixed(2)}`;
        takeProfitEl.textContent = `$${data.trading_levels.take_profit.toFixed(2)}`;
        stopLossEl.textContent = `$${data.trading_levels.stop_loss.toFixed(2)}`;
        rsiDisplayEl.textContent = `${data.indicators.rsi.toFixed(1)} / ${data.indicators.htf_rsi.toFixed(1)}`;

        // AI Verdict
        const verdict = typeof data.ai_prediction === 'string' ? data.ai_prediction : data.ai_prediction.reason;
        predictionEl.textContent = verdict || 'Keine Angabe';

        // Chart
        initChart();
        const candles = data.candles.map(c => ({
            time: c.timestamp.split(' ')[0],
            open: c.open, high: c.high, low: c.low, close: c.close
        }));
        candlestickSeries.setData(candles);
        chart.timeScale().fitContent();

        // News & Sentiment
        const newsData = data.news || [];
        const sentimentData = generateMockSentiment(ticker, newsData);
        renderNews(newsData);
        renderSentiment(sentimentData);
        renderSentimentContext(sentimentData, data.ai_prediction);

        // Risk Management Display
        renderRiskManagement(data.risk_management, data.trading_levels);

        resultsSection.style.display = 'block';
    } catch (e) {
        alert('Analyse fehlgeschlagen: ' + e.message);
    } finally {
        loadingOverlay.style.display = 'none';
    }
}

function renderRiskManagement(risk, levels) {
    const box = document.getElementById('riskManagementBox');
    const metrics = document.getElementById('riskMetrics');
    if (!risk) {
        box.style.display = 'none';
        return;
    }

    box.style.display = 'block';
    metrics.innerHTML = `
        <div class="metric-card" style="background: white; border: 1px solid var(--border-glass);">
            <div class="metric-label">Risiko Betrag</div>
            <div class="metric-value" style="font-size: 1.2rem;">$${risk.risk_amount.toFixed(2)}</div>
            <div style="font-size: 0.75rem; color: var(--text-dim); mt-1">${risk.risk_pct}% vom Konto</div>
        </div>
        <div class="metric-card" style="background: white; border: 1px solid var(--border-glass);">
            <div class="metric-label">Empf. Menge</div>
            <div class="metric-value" style="font-size: 1.2rem;">${risk.recommended_units.toFixed(risk.recommended_units < 1 ? 4 : 2)}</div>
            <div style="font-size: 0.75rem; color: var(--text-dim); mt-1">Einheiten / Kontrakte</div>
        </div>
        <div class="metric-card" style="background: white; border: 1px solid var(--border-glass);">
            <div class="metric-label">Positionsgröße</div>
            <div class="metric-value" style="font-size: 1.2rem;">$${risk.notional_value.toFixed(2)}</div>
            <div style="font-size: 0.75rem; color: var(--text-dim); mt-1">Gegenwert (Nennwert)</div>
        </div>
        <div class="metric-card" style="background: white; border: 1px solid var(--border-glass);">
            <div class="metric-label">Eff. Hebel</div>
            <div class="metric-value" style="font-size: 1.2rem;">x${risk.suggested_leverage.toFixed(1)}</div>
            <div style="font-size: 0.75rem; color: var(--text-dim); mt-1">Notional / Kapital</div>
        </div>
    `;
}

function renderNews(news) {
    const newsBox = document.getElementById('newsCards');
    newsBox.innerHTML = '';

    if (!news || news.length === 0) {
        newsBox.innerHTML = '<p class="text-dim" style="text-align:center; padding: 1rem;">Keine News verfügbar</p>';
        return;
    }

    news.slice(0, 5).forEach(n => {
        const div = document.createElement('div');
        div.className = 'activity-item';
        div.style.cursor = 'pointer';
        div.onclick = () => window.open(n.url, '_blank');

        // Determine sentiment badge
        const sentimentBadge = n.sentiment ? `
            <span class="news-sentiment-badge ${n.sentiment}">
                <i class="fa-solid fa-${n.sentiment === 'bullish' ? 'arrow-trend-up' : n.sentiment === 'bearish' ? 'arrow-trend-down' : 'minus'}"></i>
                ${n.sentiment.toUpperCase()}
            </span>
        ` : '';

        div.innerHTML = `
            <div style="flex:1;">
                <div style="display: flex; align-items: center; gap: 0.5rem; margin-bottom: 0.25rem;">
                    ${sentimentBadge}
                </div>
                <div style="font-weight:600; font-size: 0.9rem;">${n.title.substring(0, 80)}...</div>
                <div style="font-size: 0.75rem; color: var(--text-dim); margin-top: 0.25rem;">${n.source}</div>
            </div>
            <i class="fa-solid fa-external-link-alt" style="font-size: 0.8rem; color: var(--text-dim);"></i>
        `;
        newsBox.appendChild(div);
    });
}

function renderSentiment(sentimentData) {
    if (!sentimentData) return;

    const panel = document.getElementById('sentimentPanel');
    const score = sentimentData.overall_score || 0;
    const bullishCount = sentimentData.bullish_count || 0;
    const bearishCount = sentimentData.bearish_count || 0;
    const neutralCount = sentimentData.neutral_count || 0;

    // Show panel
    panel.style.display = 'block';

    // Update gauge
    const fillEl = document.getElementById('sentimentFill');
    const scoreEl = document.getElementById('sentimentScore');

    // Convert score (-1 to 1) to percentage (0 to 100)
    const fillPercent = ((score + 1) / 2) * 100;
    fillEl.style.width = `${fillPercent}%`;

    // Update score text and color
    scoreEl.textContent = score.toFixed(2);
    scoreEl.className = 'gauge-score';
    if (score > 0.2) scoreEl.classList.add('bullish');
    else if (score < -0.2) scoreEl.classList.add('bearish');
    else scoreEl.classList.add('neutral');

    // Update counts
    document.getElementById('bullishCount').textContent = bullishCount;
    document.getElementById('bearishCount').textContent = bearishCount;
    document.getElementById('neutralCount').textContent = neutralCount;
}

function renderSentimentContext(sentimentData, aiPrediction) {
    if (!sentimentData) return;

    const contextBox = document.getElementById('sentimentContext');
    const titleEl = document.getElementById('contextTitle');
    const subtitleEl = document.getElementById('contextSubtitle');
    const iconEl = contextBox.querySelector('.context-icon i');

    const score = sentimentData.overall_score || 0;

    // Determine if sentiment aligns with prediction
    let aligned = true;
    let message = '';
    let icon = 'fa-circle-check';

    // Simple alignment check (can be enhanced)
    if (score > 0.2) {
        message = 'Bullish news sentiment';
        aligned = true;
    } else if (score < -0.2) {
        message = 'Bearish news sentiment';
        aligned = true;
    } else {
        message = 'Neutral news sentiment';
        aligned = true;
    }

    // Update context box
    contextBox.style.display = 'flex';
    contextBox.className = 'sentiment-context';

    if (score > 0.2) {
        titleEl.textContent = '📈 Sentiment aligned';
        subtitleEl.textContent = `${sentimentData.bullish_count} bullish headlines support the analysis`;
    } else if (score < -0.2) {
        titleEl.textContent = '📉 Negative sentiment detected';
        subtitleEl.textContent = `${sentimentData.bearish_count} bearish headlines indicate caution`;
        contextBox.classList.add('warning');
    } else {
        titleEl.textContent = '➖ Neutral sentiment';
        subtitleEl.textContent = 'News sentiment is balanced';
    }

    iconEl.className = `fa-solid ${icon}`;
}

// Mock sentiment generator (Phase 1 - will be replaced by backend)
function generateMockSentiment(ticker, news) {
    // Generate random but realistic sentiment data
    const baseScore = Math.random() * 2 - 1; // -1 to 1
    const total = news.length;

    let bullishCount = 0;
    let bearishCount = 0;
    let neutralCount = 0;

    // Assign sentiments to news
    news.forEach(n => {
        const rand = Math.random();
        if (baseScore > 0.2) {
            // Bullish bias
            if (rand < 0.6) { n.sentiment = 'bullish'; bullishCount++; }
            else if (rand < 0.8) { n.sentiment = 'neutral'; neutralCount++; }
            else { n.sentiment = 'bearish'; bearishCount++; }
        } else if (baseScore < -0.2) {
            // Bearish bias
            if (rand < 0.6) { n.sentiment = 'bearish'; bearishCount++; }
            else if (rand < 0.8) { n.sentiment = 'neutral'; neutralCount++; }
            else { n.sentiment = 'bullish'; bullishCount++; }
        } else {
            // Neutral
            if (rand < 0.4) { n.sentiment = 'neutral'; neutralCount++; }
            else if (rand < 0.7) { n.sentiment = 'bullish'; bullishCount++; }
            else { n.sentiment = 'bearish'; bearishCount++; }
        }
    });

    return {
        overall_score: baseScore,
        bullish_count: bullishCount,
        bearish_count: bearishCount,
        neutral_count: neutralCount
    };
}

// Autocomplete Logic
function initAutocomplete() {
    tickerInput.addEventListener('input', (e) => {
        const query = e.target.value.toLowerCase();
        if (!query) { autocompleteDropdown.style.display = 'none'; return; }

        const matches = symbolDatabase.filter(s => s.symbol.toLowerCase().includes(query) || s.name.toLowerCase().includes(query));
        if (matches.length > 0) {
            autocompleteDropdown.innerHTML = matches.map(m => `
                <div class="dropdown-item" onclick="selectSymbol('${m.symbol}')">
                    <div style="font-weight:700;">${m.symbol}</div>
                    <div style="font-size:0.8rem; color:var(--text-dim);">${m.name}</div>
                </div>
            `).join('');
            autocompleteDropdown.style.display = 'block';
        } else {
            autocompleteDropdown.style.display = 'none';
        }
    });

    document.addEventListener('click', (e) => {
        if (!tickerInput.contains(e.target)) autocompleteDropdown.style.display = 'none';
    });
}

window.selectSymbol = (symbol) => {
    tickerInput.value = symbol;
    autocompleteDropdown.style.display = 'none';
};

function initQuickSymbols() {
    document.querySelectorAll('.chip').forEach(c => {
        c.addEventListener('click', () => {
            tickerInput.value = c.dataset.symbol;
            startAnalysis();
        });
    });
}

// History & Feedback
async function loadRecentAnalyses() {
    const list = document.getElementById('recentList');
    // Minimal indicator to avoid heavy flickering
    if (list.innerHTML === '') {
        list.innerHTML = '<p class="text-dim">Lade Historie...</p>';
    }

    try {
        const res = await fetch('/api/recent-analyses');
        const data = await res.json();

        // Use a document fragment or build string to minimize DOM ops
        const fragment = document.createDocumentFragment();
        data.forEach(a => {
            const card = document.createElement('div');
            card.className = 'activity-item';
            card.style.flexDirection = 'column';
            card.style.alignItems = 'stretch';
            card.style.gap = '1rem';

            card.innerHTML = `
                <div style="display:flex; justify-content:space-between; align-items:center;">
                    <div style="font-weight:700; font-size:1.1rem;">${a.ticker}</div>
                    <div style="font-size:0.8rem; color:var(--text-dim);">${a.timestamp}</div>
                </div>
                <div style="display:grid; grid-template-columns: repeat(3, 1fr); gap: 0.5rem;">
                    <div class="mini-status">E: ${a.trading_levels.entry.toFixed(1)}</div>
                    <div class="mini-status" style="color:var(--success);">T: ${a.trading_levels.take_profit.toFixed(1)}</div>
                    <div class="mini-status" style="color:var(--danger);">S: ${a.trading_levels.stop_loss.toFixed(1)}</div>
                </div>
                <div style="display:flex; justify-content:space-between; align-items:center; border-top: 1px solid var(--border-glass); padding-top: 0.75rem;">
                    ${a.feedback?.submitted ?
                    `<span style="color:var(--${a.feedback.success ? 'success' : 'danger'}); font-weight:600;">
                            <i class="fa-solid fa-${a.feedback.success ? 'check' : 'xmark'}"></i> ${a.feedback.success ? 'Gewonnen' : 'Verloren'}
                        </span>` :
                    `<div style="display:flex; gap:0.5rem;">
                            <button class="btn-micro success" onclick="submitFeedback('${a.id}', true)">Win</button>
                            <button class="btn-micro danger" onclick="submitFeedback('${a.id}', false)">Loss</button>
                        </div>`
                }
                    <button class="btn-delete" onclick="deleteAnalysis('${a.id}')">Löschen</button>
                </div>
            `;
            fragment.appendChild(card);
        });

        list.innerHTML = '';
        list.appendChild(fragment);
    } catch (e) {
        list.innerHTML = 'Fehler beim Laden';
    }
}

window.submitFeedback = async (id, success) => {
    await fetch('/api/feedback', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ analysis_id: id, success, remark: '' })
    });
    loadRecentAnalyses();
    loadDashboardStats();
};

window.deleteAnalysis = async (id) => {
    const confirmed = await showCustomModal(
        'Analyse löschen',
        'Möchten Sie diese Analyse wirklich dauerhaft aus der Historie entfernen?',
        'Löschen'
    );

    if (confirmed) {
        try {
            const res = await fetch(`/api/analysis/${id}`, { method: 'DELETE' });
            if (res.ok) {
                loadRecentAnalyses();
                loadDashboardStats();
            }
        } catch (e) {
            console.error('Löschen fehlgeschlagen', e);
        }
    }
};

analyzeBtn.addEventListener('click', startAnalysis);
window.addEventListener('resize', () => { if (chart) chart.applyOptions({ width: chartContainer.clientWidth }); });
