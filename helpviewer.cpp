#include "helpviewer.h"
#include "application.h"
#include "QsLog.h"
#include "definedsettings.h"
#include "externs.h"
HelpBrowser::HelpBrowser(QWidget * parent) : QTextBrowser(parent) {
  connect(this,SIGNAL(anchorClicked(const QUrl &)),this,SLOT(getAnchor(const QUrl &)));
}
QVariant HelpBrowser::loadResource(int type, const QUrl &name)
{
  QLOG_DEBUG() << Q_FUNC_INFO << type << name;
  //  QLOG_DEBUG() << m_he->currentFilter();
  if (type < 4) {
    QStringList regs =  m_he->registeredDocumentations();
    QList<QUrl> files;
    files = m_he->files(regs[0],m_he->filterAttributes(),"png");
    for(int i=0;i < files.size();i++) {
      if (files[i].fileName() == name.fileName()) {
        QByteArray ba = m_he->fileData(files[i]);
        return ba;
      }
    }
  }
  return QTextBrowser::loadResource(type, name);
}
void HelpBrowser::getAnchor(const QUrl & url) {
  m_firstAnchor = url.toString();
  m_firstAnchor = m_firstAnchor.remove(0,1);
  QLOG_DEBUG() << url << m_firstAnchor;
  if ((this->backwardHistoryCount() == 0) && ! m_firstAnchor.isEmpty()) {
    QLOG_DEBUG() << "emitting backward available";
    emit(backwardAvailable(true));
  }
}
void HelpBrowser::toAnchor() {
  if ( ! m_firstAnchor.isEmpty()) {
    QString c = m_firstAnchor;
    c = "j" + c.remove(0,1);
    QLOG_DEBUG() << "scroll to anchor" << m_firstAnchor << " --->"  << c;
    this->scrollToAnchor(c);
  }
}
/**
 *
 *
 * @param parent
 */
HelpViewer::HelpViewer(QWidget * parent) : QWidget(parent) {
  QVBoxLayout * layout = new QVBoxLayout;
  QHBoxLayout * btnlayout = new QHBoxLayout;

  m_backBtn = new QPushButton("Back");
  m_searchBtn = new QPushButton("Search");
  btnlayout->addWidget(m_searchBtn);
  btnlayout->addWidget(m_backBtn);
  btnlayout->addStretch();
  m_browser = new HelpBrowser;

  connect(m_browser,SIGNAL(backwardAvailable(bool)),this,SLOT(backwardAvailable(bool)));
  connect(m_backBtn,SIGNAL(clicked()),this,SLOT(goBack()));
  layout->addWidget(m_browser);
  layout->addLayout(btnlayout);
  setLayout(layout);
}
HelpViewer::~HelpViewer() {
  QLOG_DEBUG() << Q_FUNC_INFO;
}
void HelpViewer::backwardAvailable(bool available) {
  QLOG_DEBUG() << Q_FUNC_INFO << available;
}

void HelpViewer::goBack() {
  QLOG_DEBUG() << Q_FUNC_INFO;
  if (m_browser->backwardHistoryCount() == 0) {
    m_browser->toAnchor();
  }
  else {
    m_browser->backward();
  }
}
HelpWidget::HelpWidget(QWidget * parent) : QWidget(parent) {
  QLOG_DEBUG() << Q_FUNC_INFO;
  m_ok = false;
  readSettings();
  m_he = new QHelpEngine(m_helpCollection);
  m_viewer = new HelpViewer(this);
  m_viewer->browser()->setHelpEngine(m_he);
  //    QHelpContentModel *contentModel = he->contentModel();


    QHelpContentWidget *contentWidget = m_he->contentWidget();
    QHelpContentModel *contentModel =
        qobject_cast<QHelpContentModel*>(contentWidget->model());
    connect(contentModel, SIGNAL(contentsCreated()), this, SLOT(contentsCreated()));
  m_he->setupData();
    QHelpIndexModel* indexModel = m_he->indexModel();
    QHelpIndexWidget* indexWidget = m_he->indexWidget();

    QSplitter* splitter = new QSplitter();
    splitter->addWidget(contentWidget);
    //    splitter->addWidget(indexWidget);
    splitter->addWidget(m_viewer);
    splitter->setStretchFactor(0,0);
    splitter->setStretchFactor(1,1);

    indexWidget->setModel(indexModel);
    QVBoxLayout * layout = new QVBoxLayout;

    QDialogButtonBox * btns = new QDialogButtonBox(QDialogButtonBox::Close);
    layout->addWidget(splitter);
    layout->addWidget(btns);
    setLayout(layout);
   connect(contentWidget,SIGNAL(linkActivated(const QUrl &)),this,SLOT(helpLinkActivated(const QUrl &)));

   connect(btns,SIGNAL(rejected()),this,SLOT(onClose()));
   if (! m_currentUrl.isEmpty()) {
     helpLinkActivated(m_currentUrl);
   }
}
HelpWidget::~HelpWidget() {
  QLOG_DEBUG() << Q_FUNC_INFO;
  writeSettings();
}
void HelpWidget::onClose() {
  QLOG_DEBUG() << Q_FUNC_INFO;
  this->hide();
}
void HelpWidget::contentsCreated() {
  QLOG_DEBUG() << Q_FUNC_INFO;

  // get registered docs
  QLOG_DEBUG() << "Collection file" << m_he->collectionFile();
   QStringList regs =  m_he->registeredDocumentations();
  QLOG_DEBUG() << "Registered documentation" << regs;

  QLOG_DEBUG() << "Current filter" << m_he->currentFilter();
  if (regs.size() == 0) {
    QLOG_DEBUG() << "No documents registered";
    return;
  }
  /// this is test code
  QList<QStringList> fa = m_he->filterAttributeSets(regs[0]);
  QLOG_DEBUG() << "Filter attribute sets" << fa;

  QList<QUrl> files;
  files = m_he->files(regs[0],fa[0]);

  for(int i=0;i < files.size();i++) {
    QByteArray ba = m_he->fileData(files[i]);
    QLOG_DEBUG() << files[i] << ba.size();
  }

  m_he->setCurrentFilter("Lanes Lexicon 1.0");
  QStringList keywords;
  keywords << "introname" << "preface" << "lanex" << "idPreface";
  QLOG_DEBUG() << "Filter attributes" << m_he->filterAttributes();
  for(int i=0;i < keywords.size();i++) {
    QMap<QString,QUrl> links = m_he->linksForIdentifier(keywords[i]);//QLatin1String("introname"));
    QLOG_DEBUG() << "Links for ID size:" << keywords[i] << links.size();
    QMapIterator<QString, QUrl> iter(links);
    while (iter.hasNext()) {
      iter.next();
      QLOG_DEBUG() << iter.key() << iter.value();
    }
  }
  /*
  links = m_he->linksForIdentifier(QLatin1String("lanex"));
  QLOG_DEBUG() << "Links for ID size:" << links.size();
  QMapIterator<QString, QUrl> it(links);
  while (it.hasNext()) {
    it.next();
    QLOG_DEBUG() << it.key() << ": " << it.value();
  }
  */
  /// end test code
  m_he->setCurrentFilter("Lanes Lexicon 1.0");
  m_he->contentWidget()->expandAll();
  m_ok = true;
}
void HelpWidget::helpLinkActivated(const QUrl & url)  {
  QLOG_DEBUG() << Q_FUNC_INFO << url << "fragment" << url.fragment();
  QByteArray helpData = m_he->fileData(url);//.constBegin().value());
  m_viewer->browser()->setHtml(helpData);
  if (url.hasFragment()) {
    m_viewer->browser()->scrollToAnchor(url.fragment());
  }
  m_viewer->setFocus(Qt::OtherFocusReason);
  m_currentUrl = url;
}

void HelpWidget::writeSettings() {
  SETTINGS
  settings.beginGroup("Help");
  settings.setValue(SID_HELP_CURRENT_PAGE,m_currentUrl);
  settings.setValue(SID_HELP_SIZE, size());
  settings.setValue(SID_HELP_POS, pos());
}
void HelpWidget::readSettings() {
  SETTINGS
  settings.beginGroup("Help");
  m_helpCollection = settings.value(SID_HELP_COLLECTION,"lanedocs.qhc").toString();
  m_currentUrl = settings.value(SID_HELP_CURRENT_PAGE).toUrl();
  resize(settings.value(SID_HELP_SIZE, QSize(500, 700)).toSize());
  move(settings.value(SID_HELP_POS, QPoint(200, 200)).toPoint());
}
void HelpWidget::showSection(const QString & id) {
  QLOG_DEBUG() << Q_FUNC_INFO << id;
  QMap<QString,QUrl> links = m_he->linksForIdentifier(id);
  QMapIterator<QString, QUrl> iter(links);
  if (iter.hasNext()) {
      iter.next();
      QLOG_DEBUG() << iter.key() << iter.value();
      helpLinkActivated(iter.value());
  }
}
/*
 TRACE_OBJ
    QByteArray ba;
    if (type < 4) {
        const QUrl url = HelpEngineWrapper::instance().findFile(name);
        ba = HelpEngineWrapper::instance().fileData(url);
        if (url.toString().endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
            QImage image;
            image.loadFromData(ba, "svg");
            if (!image.isNull())
                return image;
        }
    }
    return ba;
*/
