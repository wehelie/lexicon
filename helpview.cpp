#include "helpview.h"
#include "application.h"
#include "externs.h"
#include "definedsettings.h"
#include "QsLog.h"
HelpView::HelpView(QWidget * parent) : QWidget(parent) {
  setObjectName("helpview");
  setWindowTitle(tr("Documentation"));
  QVBoxLayout * layout = new QVBoxLayout;
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 1))
  m_forwardButton = new QPushButton(QIcon(QPixmap(":/qrc/arrow-right.svg")),tr("Forward"));
  m_backButton = new QPushButton(QIcon(QPixmap(":/qrc/arrow-left.svg")),tr("Back"));
  m_closeButton = new QPushButton(QIcon(QPixmap(":/qrc/window-close.svg")),tr("Close"));

  connect(m_forwardButton,SIGNAL(clicked()),this,SLOT(onPageForward()));
  connect(m_backButton,SIGNAL(clicked()),this,SLOT(onPageBack()));
  connect(m_closeButton,SIGNAL(clicked()),this,SLOT(onClose()));
  QHBoxLayout * btnlayout = new QHBoxLayout;
  btnlayout->addWidget(m_backButton);
  btnlayout->addWidget(m_forwardButton);
  btnlayout->addWidget(m_closeButton);

  btnlayout->addStretch();
  layout->addLayout(btnlayout);



  m_view = new QWebView(this);

  layout->addWidget(m_view);
  setLayout(layout);
  m_initialPage = true;
  m_timer = 0;
  m_progress = 0;

  connect(m_view,SIGNAL(linkClicked(const QUrl &)),this,SLOT(linkclick(const QUrl &)));
  //  connect(btns, SIGNAL(rejected()), this, SLOT(onClose()));
  connect(m_view,SIGNAL(loadProgress(int)),this,SLOT(loadProgress(int)));
  connect(m_view,SIGNAL(loadStarted()),this,SLOT(loadStarted()));
  connect(m_view,SIGNAL(loadFinished(bool)),this,SLOT(loadFinished(bool)));
  readSettings();


#else
  QTextBrowser * b = new QTextBrowser;

  QString html = "<p>Versions of this software built with Qt 5.7.1 or newer do not provide an inbuilt documentation viewer and will try to use your browser to view the online documenation located <a href=\"http://laneslexicon.github.io/lexicon/site/\">here.</a> This may change. ";

  b->setHtml(html);

  b->setReadOnly(true);
  b->setOpenExternalLinks(true);
  m_closeButton = new QPushButton(QIcon(QPixmap(":/qrc/window-close.svg")),tr("Close"));
  connect(m_closeButton,SIGNAL(clicked()),this,SLOT(onClose()));
  QHBoxLayout * btnlayout = new QHBoxLayout;
  btnlayout->addWidget(m_closeButton);

  btnlayout->addStretch();
  layout->addWidget(b);
  layout->addLayout(btnlayout);
  setLayout(layout);
  readSettings();
  resize(QSize(400,200));
  m_localSource = false;
#endif

}
/**
 * First time through, try to make a sensible decision about what the
 * initial page should be.
 *
 * @return
 */
bool HelpView::loadHelpSystem() {
  /**
   * check the current page points to an actual file. e.g. when the saved page url is out
   * of date after documentation changes
   *
   * If there is a problem loading the page, it seems to default to about:blank, so
   * check for this as well.
   */

  if (m_localSource && ! m_currentLocalPage.isEmpty()) {
    QFileInfo cp(m_currentLocalPage.toLocalFile());
    if (! cp.exists()) {
      m_currentLocalPage.clear();
    }
  }
  //  m_localPrefix = "file:///";
  //  m_localPrefix = "http://127.0.0.1:8000/";
  QString prefix;
  QString location;
  QUrl startPage;
  if (m_localSource) {
    prefix = m_localPrefix;
    location = m_localRoot;
    startPage = m_currentLocalPage;
  }
  else {
    prefix = m_onlinePrefix;
    location = m_onlineRoot;
    startPage = m_currentOnlinePage;
  }
  if (startPage.isEmpty() || (startPage == QUrl("about:blank"))) {
    if (m_localSource) {
      if (location.endsWith(QDir::separator())) {
        location.chop(1);
      }
      QFileInfo fi(m_localRoot + QDir::separator() + "index.html");
      startPage = QUrl::fromLocalFile(fi.absoluteFilePath());
    }
    else {
      startPage = QUrl(prefix + "/" + "index.html");
    }
  }
  m_currentUrl = startPage;
  QLOG_DEBUG() << Q_FUNC_INFO << "Loading initial page" << startPage;
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 1))
  m_view->load(startPage);
  m_view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  m_forwardButton->setEnabled(false);
  m_backButton->setEnabled(false);
#else
  QLOG_DEBUG() << "desktop services" << startPage;
  QDesktopServices::openUrl(startPage);
#endif
  return true;
}
HelpView::~HelpView() {
  QLOG_DEBUG() << Q_FUNC_INFO;
  writeSettings();
}
void HelpView::linkclick(const QUrl & url) {
  QLOG_DEBUG() << Q_FUNC_INFO << url;
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 1))
  /**
   *  external links opened via QDesktopServices
   *
   */

  if (m_localSource && ! url.isLocalFile()) {
    QDesktopServices::openUrl(url);
    return;
  }
  if (! m_localSource) {
    if (url.host() !=  QUrl(m_onlinePrefix).host()) {
      QDesktopServices::openUrl(url);
      return;
    }
  }
  QString str = url.toString();
  if (str.endsWith(QDir::separator())) {
    str.chop(1);
  }
  QUrl f(str + QDir::separator() + "index.html");
  m_view->load(f);
#else
      QDesktopServices::openUrl(url);
      return;
#endif
}
void HelpView::onClose() {
  this->hide();
}
void HelpView::readSettings() {
  SETTINGS

  settings.beginGroup("Help");
  m_localSource = settings.value(SID_HELP_LOCAL,true).toBool();


  m_localRoot = settings.value(SID_HELP_LOCAL_LOCATION,"site").toString();
  m_localPrefix = settings.value(SID_HELP_LOCAL_PREFIX,"file://").toString();

  QString str = settings.value(SID_HELP_LOCAL_URL,QString()).toString();


  QDir d = QDir::current();
  d.setPath(m_localRoot);
  QFileInfo fi(d,str);
  m_currentLocalPage = QUrl::fromLocalFile(fi.absoluteFilePath());

  m_onlineRoot = settings.value(SID_HELP_ONLINE_LOCATION,QString()).toString();
  m_onlinePrefix = settings.value(SID_HELP_ONLINE_PREFIX,"http://").toString();
  m_currentOnlinePage = settings.value(SID_HELP_ONLINE_URL,QUrl()).toUrl();
  resize(settings.value(SID_HELP_SIZE,QSize(800,600)).toSize());
  move(settings.value(SID_HELP_POS,QSize(450,20)).toPoint());
}
/**
 * If viewing local docs, do not write external URL as the current
 *
 */
void HelpView::writeSettings() {
  SETTINGS
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 1))
  settings.beginGroup("Help");
  settings.setValue(SID_HELP_SIZE, size());
  settings.setValue(SID_HELP_POS, pos());

  QString localbase = settings.value(SID_HELP_LOCAL_LOCATION,"site").toString();
  //  QFileInfo fi(QDir::current(),localbase);
  QString str;
  QDir d = QDir::current();
  d.setPath(localbase);
  if (m_localSource) {
    if (m_view->url().isLocalFile()) {
      QString str = m_view->url().toLocalFile();
      settings.setValue(SID_HELP_LOCAL_URL, d.relativeFilePath(str));//m_view->url());
    }
  }
  else {
    settings.setValue(SID_HELP_ONLINE_URL, m_view->url());
  }
  settings.endGroup();
#endif
}
/**
 * The first load can sometimes take a few seconds so we are showing
 * a progress dialog
 *
 * @param x
 */
void HelpView::loadProgress(int x) {
  if (m_progress) {
    m_progress->setValue(x);
  }
}
void HelpView::loadStarted() {
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 1))

  if (m_initialPage) {
    this->showMinimized();
    m_progress = new QProgressDialog(tr("Loading ..."), tr("Cancel"), 0, 100);
    m_progress->setWindowTitle(tr("Help System"));
    //    m_progress->setCancelButton(0);
    m_timer = new QTimer(this);
    connect(m_progress, SIGNAL(canceled()), this, SLOT(onCancel()));
    m_timer->start(0);
    m_progress->show();
    m_progress->activateWindow();
  }
#endif
}
/**
 *
 *
 * @param ok
 */
void HelpView::loadFinished(bool /* ok */) {
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 1))
  QLOG_DEBUG() << Q_FUNC_INFO << m_view->url() << ok;
  if (m_initialPage) {
    this->showNormal();
    m_view->show();
    m_initialPage = false;
    m_timer->stop();
    delete m_progress;
    delete m_timer;
    m_timer = 0;
    m_progress = 0;
    if (m_stack.size() > 0) {
      m_view->load(m_stack.takeFirst());
    }
    emit(helpSystemLoaded(ok));
  }
  if (ok) {
    this->showNormal();
    m_view->show();
    m_view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    m_forwardButton->setEnabled(m_view->page()->history()->canGoForward());
    m_backButton->setEnabled(m_view->page()->history()->canGoBack());
  }
  else {
    QLOG_INFO() << "Load failed";
  }
  if (! m_initialPage ) {
    emit(finished(ok));
  }
#endif
}
bool HelpView::isLoaded() const {
  return ! m_initialPage;
}
bool HelpView::isOffline() const {
  return m_localSource;
}
void HelpView::onCancel() {
    m_initialPage = true;
    m_timer->stop();
    delete m_progress;
    delete m_timer;
    m_timer = 0;
    m_progress = 0;
    this->hide();
    emit(finished(false));
}
void HelpView::showEvent(QShowEvent * event) {
  //  qDebug() << Q_FUNC_INFO << m_initialPage;
  QWidget::showEvent(event);
}
void HelpView::onPageForward() {
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 1))
  if (m_view->page()->history()->canGoForward()) {
    m_view->page()->history()->forward();
  }
#endif
}
void HelpView::onPageBack() {
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 1))
  if (m_view->page()->history()->canGoBack()) {
    m_view->page()->history()->back();
  }
#endif
}
void HelpView::showSection(const QString & section) {
  QUrl startPage;
  QString prefix;
  QString location;

  if (m_localSource) {
    prefix = m_localPrefix;
    location = m_localRoot;
  }
  else {
    prefix = m_onlinePrefix;
    location = m_onlineRoot;
  }
  if (m_localSource) {
    if (location.endsWith(QDir::separator())) {
      location.chop(1);
    }
    QFileInfo fi(m_localRoot + QDir::separator() + section);
    QLOG_DEBUG() << Q_FUNC_INFO << "Loading" << fi.absoluteFilePath();
    startPage = QUrl(m_localPrefix + fi.absoluteFilePath());
  }
  else {
    startPage = QUrl(prefix + "/" + section);
  }
  QLOG_DEBUG() << Q_FUNC_INFO << "Loading section" << section << startPage;
  /// save the requested url so if it fails
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 1))
  m_currentUrl = startPage;
  if ( m_initialPage ) {
    m_stack << m_currentUrl;
    return;
  }

  m_view->load(startPage);
  m_view->show();
#else
  QLOG_DEBUG() << "section via desktopservices" << startPage;
  QDesktopServices::openUrl(startPage);
#endif
}
QUrl  HelpView::lastWishes() const {
  return m_currentUrl;
}
