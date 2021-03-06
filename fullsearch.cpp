#include "fullsearch.h"
#include "searchoptionswidget.h"
//#include "graphicsentry.h"
#include "application.h"
#include "laneslexicon.h"
#include "imlineedit.h"
#include "nodeview.h"
#include "columnartablewidget.h"
#include "keyboard.h"
#include "keyboardwidget.h"
#include "xsltsupport.h"
#include "centeredcheckbox.h"
#include "definedsettings.h"
#include "definedsql.h"
#include "externs.h"
#define SELECT_COLUMN 0
#define ROOT_COLUMN 1
#define HEAD_COLUMN 2
#define NODE_COLUMN 3
#define POSITION_COLUMN 4
#define VOL_COLUMN 5
#define CONTEXT_COLUMN 6
/**
 *
 *
 * @param parent
 */
FullSearchWidget::FullSearchWidget(QWidget * parent) : QWidget(parent) {
  readSettings();
  setWindowTitle(tr("Search for Arabic word"));
  setObjectName("fullsearchwidget");
  m_attached = false;
  m_findTarget = new ImLineEdit;
  m_findTarget->setObjectName("arabicedit");
  getLexicon()->adjustHeight(m_findTarget);
  QString mapname = getApp()->getActiveKeymap();
  QString mapfile = getApp()->getKeymapFileName(mapname);
  if (! mapfile.isEmpty()) {
    m_findTarget->loadMap(mapfile,mapname);
    m_findTarget->setCurrentMap(mapname);
  }
  m_mapEnabled = m_findTarget->isMappingEnabled();

  //  m_findTarget->readSettings(getLexicon()->settingsFileName());

  m_findButton = new QPushButton(tr("F&ind"));
  m_findButton->setDefault(true);
  m_hideOptionsButton = new QPushButton(tr("Sho&w options"));
  //  m_hideOptionsButton->setCheckable(true);

  m_keyboardButton  = new QPushButton(tr("Show &keyboard"));
  m_keyboardButton->setAutoDefault(false);
  m_keyboardButton->setCheckable(true);

  m_keyboard = new KeyboardWidget(getLexicon()->getResourceFilePath(Lexicon::Keyboard),m_keyboardConfig,this);
  connect(m_findButton,SIGNAL(clicked()),this,SLOT(findTarget()));
  connect(m_hideOptionsButton,SIGNAL(clicked()),this,SLOT(hideOptions()));
  connect(m_keyboardButton, SIGNAL(clicked()),this,SLOT(showKeyboard()));


  QHBoxLayout * targetlayout = new QHBoxLayout;
  targetlayout->addWidget(m_findTarget);
  targetlayout->addWidget(m_findButton);
  targetlayout->addWidget(m_keyboardButton);
  targetlayout->addWidget(m_hideOptionsButton);


  m_defaultOptions.setSearchScope(SearchOptions::Word);
  m_search = new SearchOptionsWidget(m_defaultOptions,this);

  connect(m_search,SIGNAL(force(bool)),m_findTarget,SLOT(setForceLTR(bool)));
  connect(m_search,SIGNAL(onLanguageSwitch(int)),this,SLOT(languageSwitch(int)));
  //  QWidget * container = new QWidget;
  m_container = new QVBoxLayout;
  QStringList headings;
  headings << tr("Mark") << tr("Root") << tr("Entry") << tr("Node") << tr("Occurs")  << tr("Vol/Page") << tr("Context");
  if (! m_singleRow) {
    headings[POSITION_COLUMN] = tr("Position");
  }
  m_rxlist = new ColumnarTableWidget(headings);

  m_rxlist->setKey(ColumnarTableWidget::STATE,SID_FULLSEARCH_LIST_STATE);
  m_rxlist->setKey(ColumnarTableWidget::COLUMN_WIDTHS,SID_FULLSEARCH_LIST_COLUMNS);
  m_rxlist->setDefaultWidth(100);
  m_rxlist->setObjectName("fullsearchresultlist");
  m_rxlist->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_rxlist->setSelectionMode(QAbstractItemView::SingleSelection);
  m_rxlist->setMarkColumn(SELECT_COLUMN);
  m_rxlist->setExportIgnore(CONTEXT_COLUMN,m_headText);
  m_rxlist->setFixedRowHeight(m_rowHeight);
  m_rxlist->installEventFilter(this);
  //  m_rxlist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  m_progress = new QProgressBar;
  m_progress->hide();
  QHBoxLayout * resultslayout = new QHBoxLayout;
  m_resultsText = new QLabel("");
  m_resultsText->hide();
  m_exportButton = new QPushButton(tr("Export results"));
  resultslayout->addWidget(m_resultsText);
  resultslayout->addStretch();
  resultslayout->addWidget(m_exportButton);
  m_container->addLayout(targetlayout);

  m_container->addWidget(m_search);
  m_spacer = new QSpacerItem(0, 20,QSizePolicy::Ignored, QSizePolicy::MinimumExpanding);
  m_container->addWidget(m_progress);
  m_container->addSpacerItem(m_spacer);

  m_container->addWidget(m_rxlist);
  m_container->addLayout(resultslayout);

  setLayout(m_container);
  connect(m_exportButton,SIGNAL(clicked()),this,SLOT(onExport()));
  connect(m_rxlist,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
          this,SLOT(itemDoubleClicked(QTableWidgetItem * )));
  connect(m_rxlist,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(onCellDoubleClicked(int,int)));
  initXslt();
  //  m_search->setOptions(m_defaultOptions);
  m_search->setVisible(false);


  SETTINGS
  settings.beginGroup("FullSearch");
  m_rxlist->readConfiguration(settings);
  //  m_rxlist->hide();

  this->setFocus();
}
FullSearchWidget::~FullSearchWidget() {
  QLOG_DEBUG() << Q_FUNC_INFO;
}
void FullSearchWidget::onCellDoubleClicked(int row,int /* col */) {
  QLOG_DEBUG() << Q_FUNC_INFO << row;

  this->showNode(row);
}
void FullSearchWidget::itemDoubleClicked(QTableWidgetItem * item) {
  QLOG_DEBUG() << Q_FUNC_INFO;
  this->showNode(item->row());
}
void FullSearchWidget::showNode(int row) {
  QLabel * label = qobject_cast<QLabel *>(m_rxlist->cellWidget(row,NODE_COLUMN));
  int pos;
  if (! label) {
    return;
  }
  QString node = label->text();

  if (node.isEmpty()) {
    QLOG_WARN() << QString("No node row %1").arg(row);
    return;
  }
  bool isHead = label->property("HEADWORD").toBool();

  m_nodeQuery.bindValue(0,node);
  m_nodeQuery.exec();
  /// missing node
  if ( ! m_nodeQuery.first()) {
    QLOG_WARN() << Q_FUNC_INFO << "No record for node" << node;
    return;
  }
  Place p = Place::fromEntryRecord(m_nodeQuery.record());

  if (m_singleRow) {
    pos = 0;
  }
  else {
    pos = label->property("TEXTINDEX").toInt();
  }
  QString xml = m_nodeQuery.value("xml").toString();
  QString html = this->transform(xml);
  NodeView * v = new NodeView(this);
  v->setAttribute(Qt::WA_DeleteOnClose);

  v->setWindowTitle(QString(tr("Showing result %1 in search for %2")
                            .arg(row + 1)
                            .arg(m_target)));
  v->setPattern(m_currentRx);
  v->setCSS(m_currentCSS);
  /**
   * get the page, check it is for a valid volume and pass it if it is, otherwise passs 0
   *
   */

  int page = m_nodeQuery.value("page").toInt();
  if (Place::volume(page) == 0) {
    page = 0;
  }
  v->setHeader(p.root(),p.word(),node,page);
  /**
   * set the index for which occurrence to show first time through
   *
   */

  v->setStartPosition(pos);
  v->setHtml(html);
  v->findFirst();
  v->show();
  v->raise();
  v->activateWindow();
  connect(v,SIGNAL(openNode(const QString &)),this,SLOT(openNode(const QString &)));
  connect(v,SIGNAL(printNode(const QString &)),this,SIGNAL(printNode(const QString &)));
}
bool FullSearchWidget::eventFilter(QObject * target,QEvent * event) {
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);

    switch(keyEvent->key()) {
    case Qt::Key_Tab: {
      m_findTarget->setFocus();
      break;
    }
    case Qt::Key_Escape: {
      QWidget * w = qobject_cast<QWidget *>(target);
      while(w) {
        QTabWidget * tabw = qobject_cast<QTabWidget *>(w);
        if (tabw)  {
          tabw->tabBar()->setFocus();
          return true;
        }
        w = w->parentWidget();
      }
      break;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space: {
      //        if (keyEvent->modifiers() && Qt::ControlModifier) {
      //      int currentRow = m_rxlist->currentRow();
      //      QTableWidgetItem * item = m_rxlist->item(currentRow,NODE_COLUMN);
      //      if (item)
      //        m_rxlist->itemDoubleClicked(item);
      this->onCellDoubleClicked(m_rxlist->currentRow(),NODE_COLUMN);
      break;
    }
    case Qt::Key_E: {
      if (keyEvent->modifiers() && Qt::ControlModifier) {
        //          if (target == m_tree)
        //          m_tabs->currentWidget()->setFocus();
        return true;
      }
      break;
    }
    default:
      break;
    }
  }
  return QWidget::eventFilter(target,event);
}
void FullSearchWidget::setSearch(const QString & searchFor,const SearchOptions & options) {
  m_target = searchFor;
  m_search->setOptions(options);
  m_findTarget->setText(m_target);
}
void FullSearchWidget::hideOptions() {
  QLOG_DEBUG() << Q_FUNC_INFO << m_search->isVisible();
  m_search->hideTabOptions();
  if ( m_search->isVisible()) {
    m_search->hide();
    m_search->setVisible(false);
    m_hideOptionsButton->setText("Show options");
  }
  else {
    m_search->setVisible(true);
    m_search->showMore(true);
    m_search->show();
    m_hideOptionsButton->setText("Hide options");
  }
}
/**
 *  for regex search do this:
 *
 TODO Check for regex special characters in buckwalter transliteration

QString escaped = pattern;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.prepend("\"");
    escaped.append("\"");

 */
void FullSearchWidget::findTarget(bool showProgress) {
  QLOG_DEBUG() << Q_FUNC_INFO << showProgress;
  m_showProgressDialog = showProgress;
  SearchOptions options;
  m_search->getOptions(options);
  /// this shows text in progressbar
#ifdef __APPLE__
  m_progress->setStyle(QStyleFactory::create("Fusion"));
#endif
  m_progress->setValue(0);
  if (! showProgress) {
    m_progress->show();
  }
  else {
    //    m_hideOptionsButton->setChecked(true);
    //    this->hideOptions();
  }
  /// TODO check if only arabic characters
  ///      and use appropriate
  QString t = m_findTarget->text();
  t.remove(QChar(0x202d));

  QRegExp rx("[a-z]+");
  // move code that builds regex for arabic to here
  // allow for english word search: standard or regex
  // and pass the pattern and a possibly empty regex to the search functions, so that when the regex is empty
  // it does a QTextDocument->find(QString) otherwise QTextDocument->find(QRegExp)
  // arabic searches that include diacritics should be treated as string search not regex
  // build a QTextDocument::FindFlags
  // if possible, remove regexSearch code
  if ((options.getSearchType() == SearchOptions::Regex) && (rx.indexIn(t,0) != -1)) {
    this->regexSearch(t,options);
  }
  else {
    this->textSearch(m_findTarget->text(),options);

  }
  m_progress->hide();

}
/**
 *
 *
 * @param entryCount the number of unique entries i.e. nodes
 * @param bodyCount  the number of occurrences within XML
 * @param options
 *
 * @return
 */
QString FullSearchWidget::buildText(int entryCount,int headCount,int bodyCount,int ms) {
  QString t;
  QString p1;
  QString p2;
  m_findCount = bodyCount + headCount;

  QString targetText;
  if (UcdScripts::isScript(m_target,"Arabic")) {
    targetText = getLexicon()->spanArabic(m_target,"fullsearchresults");
  }
  else {
    targetText = m_target;
  }

  if (m_findCount == 0) {
    t = QString(tr("Search \"%1\": no items found")).arg(targetText).arg(p1);
  }
  else {
    t = QString(tr("Search \"%1\": found %2")).arg(targetText).arg(m_findCount);

    t += (m_findCount > 1 ? QString(tr(" matches")) : QString(tr("match")));
    t += (m_findCount > 1 ? QString(tr(" in %1 entries")).arg(entryCount) : QString(tr(" in 1 entry")));

    if (headCount > 0) {
      t += (headCount > 1 ? QString(tr( " and %1 headwords")).arg(headCount) : QString(tr(" and 1 headword")));
    }
  }
  if (m_searchOptions.getSearchType() == SearchOptions::Regex) {
    t += tr(", regular expression search");
  }
  else {
    if (m_searchOptions.textSearch()) {
      if (m_searchOptions.ignoreDiacritics())
        t += tr(", ignoring diacritics");
      if (m_searchOptions.isWholeWord())
        t += tr(", whole word match");
    }
    else {
      if (m_searchOptions.ignoreCase())
        t += tr(", ignoring case");
      if (m_searchOptions.isWholeWord())
        t += tr(", whole word match");
    }
  }
  if (ms != -1) {
    qreal x = (ms/1000) + 0.5;
    int y = static_cast<int>(x);
    t += QString(" (%1 sec").arg(y);
    if (y != 1) {
      t += "s";
    }
    t += ")";
  }
  return t;
}
/**
 *
 *
 * @param record
 * @param text
 * @param pos index to the m_positions array of text positions or the actual position
 *
 * @return
 */
int FullSearchWidget::addRow(const Place & p, const QString & text,int pos) {

  QTableWidgetItem * item;
  QLabel * label;

  int row = m_rxlist->rowCount();
  m_rxlist->insertRow(row);

  m_rxlist->setCellWidget(row,SELECT_COLUMN,new CenteredCheckBox);

  label = new QLabel(qobject_cast<Lexicon *>(qApp)->scanAndStyle(p.root(),"fullsearchlist"));
  label->setAlignment(Qt::AlignCenter);
  m_rxlist->setCellWidget(row,ROOT_COLUMN,label);

  label = m_rxlist->createLabel(p.head(),"fullsearchlist");
  label->setAlignment(Qt::AlignCenter);
  m_rxlist->setCellWidget(row,HEAD_COLUMN,label);

  label = m_rxlist->createLabel(p.node());
  label->setAlignment(Qt::AlignCenter);
  m_rxlist->setCellWidget(row,NODE_COLUMN,label);

  int c = pos;
  ///
  /// for pos = -1
  ///
  if (pos == -1) {
    m_rxlist->setCellWidget(row,POSITION_COLUMN,new QLabel(""));
  }
  else {
    if (! m_singleRow ) {
      c = 0;
      if ((pos >= 0) && (pos < m_positions.size())) {
        c = m_positions[pos];
      }
    }
    /// set the position on the node label for later use
    label->setProperty("TEXTINDEX",pos);
    ///
    label = m_rxlist->createLabel((QString("%1").arg(c)));
    label->setAlignment(Qt::AlignCenter);
    m_rxlist->setCellWidget(row,POSITION_COLUMN,label);
  }

  label = m_rxlist->createLabel(text,"fullsearchlist");
  if (text == m_headText) {
    label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  }
  else {
    if (this->startsWithArabic(text)) {
      label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    }
    else {
      label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    }
  }
  if ( ! m_contextStyle.isEmpty()) {
    QString str = label->text();
    str = QString("<span style=\"%1\">%2</span>").arg(m_contextStyle).arg(str);
    label->setText(str);
  }

  m_rxlist->setCellWidget(row,CONTEXT_COLUMN,label);
  label = m_rxlist->createLabel(p.format("%V/%P"));
  label->setAlignment(Qt::AlignCenter);
  m_rxlist->setCellWidget(row,VOL_COLUMN,label);

  return row;
}
QTextDocument * FullSearchWidget::fetchDocument(const QString & xml) {
  QString html = transform(xml);
  m_nodeDoc.setHtml(html);
  QTextDocument * doc = new QTextDocument;
  doc->setHtml(html);
  return doc;
}
QString FullSearchWidget::transform(const QString & xml) {
  int ok = compileStylesheet(NODE_XSLT,m_xsltSource);
  if (ok == 0) {
    QString html = xsltTransform(NODE_XSLT,xml);
    if (! html.isEmpty()) {
      return html;
    }
  }
  /// could be errors in stylesheet or in the xml
  QStringList errors = getParseErrors();
  if (ok != 0) {
    errors.prepend("Errors when processing stylesheet:");
  }
  else {
    errors.prepend("Errors when processing entry:");
  }
  QMessageBox msgBox;
  msgBox.setText(errors.join("\n"));
  //  msgBox.exec();
  clearParseErrors();
  return QString();
}
void FullSearchWidget::readSettings() {
  QString v;
  SETTINGS
  settings.beginGroup("Entry");
  QString css = settings.value(SID_ENTRY_CSS,QString("entry.css")).toString();
  css = getLexicon()->getResourceFilePath(Lexicon::Stylesheet,css);
  readCssFromFile(css);
  //m_xsltSource = settings.value("XSLT",QString("entry.xslt")).toString();
  settings.endGroup();
  settings.beginGroup("FullSearch");
  m_contextStyle = settings.value(SID_FULLSEARCH_CONTEXT_STYLE,QString()).toString();
  m_singleRow = settings.value(SID_FULLSEARCH_ONE_ROW,true).toBool();
  QString f = settings.value(SID_FULLSEARCH_RESULTS_FONT,QString()).toString();
  if (! f.isEmpty()) {
    m_resultsFont.fromString(f);
  }
  m_resizeRows = settings.value(SID_FULLSEARCH_RESIZE_ROWS,true).toBool();
  m_rowHeight  = settings.value(SID_FULLSEARCH_ROW_HEIGHT,40).toInt();;

  m_debug = settings.value(SID_FULLSEARCH_DEBUG,false).toBool();
  m_fragmentSize = settings.value(SID_FULLSEARCH_FRAGMENT_SIZE,40).toInt();

  m_defaultOptions.setIncludeHeads(settings.value(SID_FULLSEARCH_INCLUDE_HEADS,false).toBool());
  m_stepCount = settings.value(SID_FULLSEARCH_STEP,500).toInt();
  v = settings.value(SID_FULLSEARCH_HEAD_BACKGROUND).toString();
  m_headBackgroundColor.setNamedColor(v);
  m_headText = settings.value(SID_FULLSEARCH_HEAD_TEXT).toString();
  if (settings.value(SID_FULLSEARCH_TYPE_REGEX,false).toBool()) {
    m_defaultOptions.setSearchType(SearchOptions::Regex);
  }
  else {
    m_defaultOptions.setSearchType(SearchOptions::Normal);
  }
  m_defaultOptions.setIgnoreDiacritics(settings.value(SID_FULLSEARCH_DIACRITICS,true).toBool());
  m_defaultOptions.setWholeWordMatch(settings.value(SID_FULLSEARCH_WHOLE_WORD,false).toBool());
  settings.endGroup();


  settings.beginGroup("XSLT");
  m_xsltSource = settings.value(SID_XSLT_ENTRY,QString("entry.xslt")).toString();
  m_xsltSource = getLexicon()->getResourceFilePath(Lexicon::XSLT,m_xsltSource);
  settings.endGroup();

  settings.beginGroup("Diacritics");
  QStringList keys = settings.childKeys();
  QStringList points;
  for(int i=0;i < keys.size();i++) {
    if (keys[i].startsWith("Char")) {
      v = settings.value(keys[i],QString()).toString();
      points << v;
    }
  }
  m_diacritics = QString("[\\x%1]*").arg(points.join("\\x"));

  settings.endGroup();
  settings.beginGroup("Keyboards");
  m_keyboardConfig = settings.value("Config","keyboard.ini").toString();

}
void FullSearchWidget::getTextFragments(QTextDocument * doc,const QString & target,const SearchOptions & options,const QRegExp & /*regex */) {
  QRegExp rx;
  QRegularExpression leadingSpaces("^\\s+");
  QRegularExpression lineBreaks("\\r|\\n");
  //  QString pattern;
  //  QRegExp rxclass(m_diacritics);
  QLOG_DEBUG() << "target" << target;
  if (options.getSearchType() == SearchOptions::Regex) {
    QLOG_DEBUG() << "regex search";
    rx.setPattern(target);
  }
  else { //if (options.textSearch()) {
    rx = SearchOptionsWidget::buildRx(target,m_diacritics,options);
  }



  m_fragments.clear();
  m_positions.clear();
  int position = 0;
  QTextCursor c;

  QTextDocument::FindFlags ff;
  // textSearch is doing two things that need to be separated
  // textSearch with arabic text will be a regex if ignore diacritics is set
  if ((options.getSearchType() == SearchOptions::Regex) || ! options.textSearch()) {
     c = doc->find(rx,position);
  }
  else {
    if (! options.ignoreCase()) {
      ff = QTextDocument::FindCaseSensitively;
    }
    if (options.wholeWordMatch()) {
      ff |= QTextDocument::FindWholeWords;
    }
    c = doc->find(target,position,ff);
  }
  QString src = doc->toPlainText();
  qDebug() << src;
  int sx;
  int ex;
  int sz = m_fragmentSize;
  while(! c.isNull()) {
    //    QLOG_DEBUG() << "cursor pos" << c.position() <<  "offset in string" << src.indexOf(rx,position);
    position = c.position();
    if (position > sz)
      sx = position - sz;
    else
      sx = 0;

    if ((position + sz) > src.size()) {
      ex = src.size();
    }
    else {
      ex = position + sz;
    }
    m_positions << position;


    QString str = src.mid(sx,ex - sx);
    str = str.remove(leadingSpaces);
    str = str.remove(lineBreaks);
    ///
    /// prepend left-to-embedding, otherwise any text starting with Arabic will display wrong
    //
    str = "‪" + str;
    m_fragments << str;
    if (!options.textSearch() || (options.getSearchType() == SearchOptions::Regex)) {
      c = doc->find(rx,position);
    }
    else {
      c = doc->find(target,position,ff);
    }
  }
  QLOG_DEBUG() << Q_FUNC_INFO << "fragments" << m_fragments.size();
  //  if (m_positions.size() > 0) {
  //    QLOG_DEBUG() << Q_FUNC_INFO << m_positions;
  //  }
}
int FullSearchWidget::getMaxRecords(const QString & table) {
  bool ok = false;
  QString sql = QString(SQL_FIND_MAXIMUM).arg(table);
  QSqlQuery maxq(sql);
  int max;
  if (maxq.exec() && maxq.next())
    max = maxq.value(0).toInt(&ok);

  if (! ok ) {
    QLOG_DEBUG() << QString("Error reading max records for %1").arg(table);
    if (table == "xref") {
      max = 544695;
    }
    else {
      max = 49000;
    }
  }
  QLOG_DEBUG() << QString("Max records for %1 : %2").arg(table).arg(max);
  return max;
}

/**
 *
 */
bool FullSearchWidget::readCssFromFile(const QString & name) {
  QFile f(name);
  if (! f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QLOG_WARN()  << QString(tr("Error reading CSS file :%1, error: %2 ")).arg(name).arg(f.errorString());
    return false;

  }
  QTextStream in(&f);
  QString css = in.readAll();
  f.close();
  if (! css.isEmpty()) {
    m_currentCSS = css;
  }
  return true;
}
void FullSearchWidget::selectFocus() {
    if (m_rxlist->rowCount() > 0) {
      m_rxlist->setFocus(Qt::OtherFocusReason);

    }
    else {
      m_findTarget->setFocus(Qt::OtherFocusReason);
    }
}
void FullSearchWidget::focusInEvent(QFocusEvent * event) {
  QLOG_DEBUG() << Q_FUNC_INFO << event;
  this->selectFocus();
  QWidget::focusInEvent(event);
}
void FullSearchWidget::focusOutEvent(QFocusEvent * event) {
  QLOG_DEBUG() << Q_FUNC_INFO << event;
  /*
  if (event->reason() == Qt::OtherFocusReason) {
    if (m_rxlist->rowCount() > 0)
      m_rxlist->setStyleSheet("QTableView { selection-background-color : green}"); //%1}").arg("lightgray"));
    m_rxlist->repaint();
  }
  */
  QWidget::focusOutEvent(event);
}
void FullSearchWidget::focusTable() {
  m_rxlist->setFocus();
}
void FullSearchWidget::cancelSearch() {
  m_cancelSearch = true;
}
void FullSearchWidget::showKeyboard() {
  if (! m_attached) {
    m_keyboard->attach(m_findTarget);
    m_keyboardButton->setText(tr("Hide &keyboard"));
    QPoint p;
    p = m_keyboard->currentPosition();
    if (p.isNull()) {
      p = m_findTarget->pos();
      p = m_findTarget->mapToGlobal(p);
      p.setX(p.x() - 50);
      p.setY(p.y() + 50);
    }
    m_keyboard->autoScale();
    m_keyboard->move(p);
    m_attached = true;
  }
  else {
    m_keyboard->detach();
    m_keyboardButton->setText(tr("Show &keyboard"));
    m_attached = false;
  }
}
/**
 * There are two ways of doing the search if ignore diacritics are set
 *    (1)  build a regex with with each with each character in the search string
 *         followed by an optional character class of all the diacritics
 *
 *    (2)  For each search word from the db, replace all characters from the
 *         the diacritics class by an empty string and then do the search
 *         i.e strip the diacritics and then attempt to match
 *
 *    In the code below, if replaceSearch is true, the 2nd method is used
 *    otherwise the first.
 *
 *  On my timings there second method adds about 200ms.
 * @param target
 * @param options
 */
void FullSearchWidget::textSearch(const QString & target,const SearchOptions & options) {
  bool replaceSearch = true;
  QLOG_DEBUG() << Q_FUNC_INFO << target << "textSearch" << options.textSearch();
  m_target = target;
  m_searchOptions = options;
  QRegExp rx;
  QRegExp rxclass(m_diacritics);
  qint64 fStart = QDateTime::currentMSecsSinceEpoch();

  QString pattern;
  m_currentRx = rx = SearchOptionsWidget::buildRx(target,m_diacritics,options);
  //
  // for arabic searches we lookup the entry in the xref table first and then
  // if the pattern matches, fetch the corresponding full entry record
  // m_nodeQuery looks up the entry from node saved in the xref table

  if (! m_nodeQuery.prepare(SQL_FIND_ENTRY_DETAILS)) {
    QLOG_WARN() << QString("SQL prepare error %1 : %2")
      .arg(SQL_FIND_ENTRY_DETAILS)
      .arg(m_nodeQuery.lastError().text());
  }
  // m_query is the loop-driver
  if (! options.textSearch()) {
    if (! m_query.prepare(SQL_FIND_XREF_ENTRIES)) {
      QLOG_WARN() << QString("SQL prepare error %1 : %2")
        .arg(SQL_FIND_XREF_ENTRIES)
        .arg(m_query.lastError().text());
      return;
    }
  }
  else {
    if (! m_query.prepare(SQL_ALL_ENTRIES)) {
      QLOG_WARN() << QString("SQL prepare error %1 : %2")
        .arg(SQL_ALL_ENTRIES)
        .arg(m_query.lastError().text());
      return;
    }
  }
  m_rxlist->clearContents();
  m_rxlist->setRowCount(0);
  m_resultsText->hide();

  int rowsLoaded = 0;
  int headCount = 0;
  int textCount = 0;
  int readCount = 0;
  int entryCount = 0;
  QString node;
  QString root;
  QString headword;
  int page;
  /// Added QEventLoop because under OSX nothing was shown
  /// the loop was finished
  QEventLoop ep;
  QProgressDialog * pd = 0;
  int max;
  if (! options.textSearch()) {
     max = this->getMaxRecords("xref");
  }
  else {
     max = this->getMaxRecords("entry");
  }

  if (m_showProgressDialog) {
    pd = new QProgressDialog("Searching...", "Cancel", 0,max, getApp());
    connect(pd,SIGNAL(canceled()),this,SLOT(cancelSearch()));
    pd->setWindowModality(Qt::WindowModal);
    pd->show();
  }

  m_progress->setMaximum(max);
  m_cancelSearch = false;
  m_query.exec();
  m_rxlist->setUpdatesEnabled(false);
  m_rxlist->setSortingEnabled(false);
  m_rxlist->blockSignals(true);
  qint64 st = QDateTime::currentMSecsSinceEpoch();
  QString xml;
  // set max rows from the command line and settings.ini
  // loading large tables can take a long time
  int maxRows = 100;
  int totalReadCount = 0;
  while(m_query.next() && ! m_cancelSearch) {
    //    QLOG_DEBUG() << m_query.executedQuery();
    readCount++;
    totalReadCount++;
    if ((readCount % m_stepCount) == 0) {
      m_progress->setValue(readCount);
      if (pd) {
        pd->setValue(readCount);
      }
      ep.processEvents();
    }
    page = m_query.value("page").toInt();
    xml.clear();
    if (! options.textSearch()) {
      QString word = m_query.value("word").toString();
      /// strip diacritics if required
      if (replaceSearch && (options.getSearchType() == SearchOptions::Normal)) {
        if (options.ignoreDiacritics())
          word =  word.replace(rxclass,QString());
      }
      if ((word.indexOf(rx) != -1) && (node != m_query.value("node").toString())) {
        node = m_query.value("node").toString();
        QLOG_DEBUG() << "Found in node" << node;
        m_nodeQuery.bindValue(0,node);
        totalReadCount++;
        if ( m_nodeQuery.exec() &&  m_nodeQuery.first()) {
          root = m_nodeQuery.value("root").toString();
          headword = m_nodeQuery.value("word").toString();
          /// strip diacritics if required
          if (replaceSearch) {
            if (options.ignoreDiacritics())
              headword =  headword.replace(rxclass,QString());
          }
          if (headword.indexOf(rx) != -1) {
            if (options.includeHeads()) {   // add headword row if required
              Place p = Place::fromEntryRecord(m_nodeQuery.record());
              int row = addRow(p,m_headText,-1);
              QLabel * label = qobject_cast<QLabel *>(m_rxlist->cellWidget(row,NODE_COLUMN));
              if (label) {
                label->setProperty("HEADWORD",true);
              }
              headCount++;
            }
          }
          headword = m_nodeQuery.value("word").toString();
          xml = m_nodeQuery.value("xml").toString();
        }
      }
      else {
      // not found in 'word'
      }
    }
    else {
      xml = m_query.value("xml").toString();
    }
    if (xml.length() > 0) {
      QTextDocument * doc  = fetchDocument(xml);
      if (doc->characterCount() > 0) {
        Place p;
        if (! options.textSearch()) {
          p = Place::fromEntryRecord(m_nodeQuery.record());
        }
        else {
          p = Place::fromEntryRecord(m_query.record());
        }
        getTextFragments(doc,target,options);
        if (m_fragments.size() > 0) {
          entryCount++;
          if (m_singleRow) {
            addRow(p,m_fragments[0],m_fragments.size());
          }
          else {
            for(int i=0;i < m_fragments.size();i++) {
              if (m_fragments[i].size() > 0) {
                addRow(p,m_fragments[i],i);
              }
            }
          }
          textCount += m_fragments.size();
        }
      }
      delete doc;
    }
    if (m_rxlist->rowCount() >= maxRows) {
      m_cancelSearch = true;
    }
  }
  QLOG_DEBUG() << QString("Read finished : %1 ms").arg(QDateTime::currentMSecsSinceEpoch() - fStart);
  QLOG_DEBUG() << QString("Total reads %1, main dataset reads %2").arg(totalReadCount).arg(readCount);
  qint64 et = QDateTime::currentMSecsSinceEpoch();

  ep.processEvents();
  ep.exit();
  m_resultsText->setText(buildText(entryCount,headCount,textCount,et - st));
  m_resultsText->show();
  QLOG_DEBUG() << QString("Set results text : %1 ms").arg(QDateTime::currentMSecsSinceEpoch() - fStart);
  m_rxlist->setUpdatesEnabled(true);
  QLOG_DEBUG() << QString("Updates enabled : %1 ms").arg(QDateTime::currentMSecsSinceEpoch() - fStart);
  if (m_rxlist->rowCount() > 0) {
    ////
    ///
    /// THIS makes the horizontal scrollbar appear. WTF
    ///
    ///
    m_rxlist->resizeColumnToContents(CONTEXT_COLUMN);
    QLOG_DEBUG() << QString("Table resize columns time : %1 ms").arg(QDateTime::currentMSecsSinceEpoch() - fStart);
    if (m_resizeRows) {
      m_rxlist->resizeRowsToContents();
      QLOG_DEBUG() << QString("Table resize rows time : %1 ms").arg(QDateTime::currentMSecsSinceEpoch() - fStart);
    }

    m_exportButton->setEnabled(true);
    m_container->removeItem(m_spacer);
    m_rxlist->show();
    QLOG_DEBUG() << QString("Table show time : %1 ms").arg(QDateTime::currentMSecsSinceEpoch() - fStart);
    m_rxlist->selectRow(0);
    m_rxlist->setFocus(Qt::OtherFocusReason);

  }
  else {
    m_findTarget->setFocus(Qt::OtherFocusReason);
    m_exportButton->setEnabled(false);
  }
  if (pd) {
    delete pd;
  }
  m_rxlist->blockSignals(false);
  QLOG_DEBUG() << QString("Function exec : %1 ms").arg(QDateTime::currentMSecsSinceEpoch() - fStart);
}

void FullSearchWidget::regexSearch(const QString & target,const SearchOptions & options) {
  QLOG_DEBUG() << Q_FUNC_INFO;
  m_target = target;
  m_searchOptions = options;


  QRegExp rx;

  QRegExp rxclass(m_diacritics);
  QString pattern;
  if (! options.textSearch()) {
    if (options.getSearchType() == SearchOptions::Normal) {
      if (options.ignoreDiacritics()) {
        m_target = m_target.replace(QRegExp(rxclass),QString());
        for(int i=0;i < target.size();i++) {
          QChar sp = m_target.at(i);
          pattern += QString(sp);
          if ( sp.isLetter() ) {
            /// if it's in the Arabic block, append to allow for optional diacritics
            if ((sp.unicode() >= 0x600) && (sp.unicode() <= 0x6ff)) {
              pattern += m_diacritics;
            }
          }
        }
      }
      else {
        pattern = target;
      }
      if (options.wholeWordMatch()) {
        pattern = "\\b" + m_target + "\\b";
      }
      rx.setPattern(pattern);
    }
    else {
      rx.setPattern(target);
    }
  }
  else {                           // English or mixed
    rx.setPattern(target);
  }
  QLOG_DEBUG() << Q_FUNC_INFO << "regex pattern" << rx.pattern();
  m_currentRx = rx;
  if (options.textSearch()) {
  QString sql(SQL_REGEX_FIND_ENTRY_DETAILS);
  if (m_query.prepare(sql)) {
    sql = SQL_FIND_ENTRY_DETAILS;// "select root,word,xml,page from entry where datasource = 1 and nodeid = ?";
    if (! m_nodeQuery.prepare(sql)) {
      QLOG_WARN() << QString("SQL prepare error %1 : %2")
        .arg(sql)
        .arg(m_nodeQuery.lastError().text());
      return;
    }
  }
  else {
    QLOG_WARN() << QString("SQL prepare error %1 : %2")
      .arg(sql)
      .arg(m_query.lastError().text());
    return;
  }
  }
  else {
    QString sql(SQL_ALL_ENTRIES);
    if (m_query.prepare(sql)) {
    }
    else {
      QLOG_WARN() << QString("SQL prepare error %1 : %2")
        .arg(sql)
        .arg(m_query.lastError().text());
      return;
    }
  }
  if (m_debug) {
    m_rxlist->showColumn(NODE_COLUMN);
  }
  else {
    m_rxlist->hideColumn(NODE_COLUMN);
  }
  //  m_rxlist->showColumn(4);
  m_rxlist->clearContents();
  m_rxlist->setRowCount(0);
  m_resultsText->hide();

  //  m_nodquery.bindValue(0,m_target);


  int headCount = 0;
  int textCount = 0;
  int readCount = 0;
  int entryCount = 0;
  QString node;
  QString root;
  QString headword;
  int page;
  /// Added QEventLoop because under OSX nothing was shown
  /// the loop was finished
  QEventLoop ep;
  QProgressDialog * pd = 0;
  int max = this->getMaxRecords("entry");
  m_progress->setMaximum(max);
  if (m_showProgressDialog) {
    pd = new QProgressDialog("Searching...", "Cancel", 0,max, getApp());
    connect(pd,SIGNAL(canceled()),this,SLOT(cancelSearch()));
    pd->setWindowModality(Qt::WindowModal);
    pd->show();
  }
  m_cancelSearch = false;
  m_query.exec();
  qint64 st = QDateTime::currentMSecsSinceEpoch();
  m_rxlist->setUpdatesEnabled(false);
  while(m_query.next() && ! m_cancelSearch) {
    readCount++;
    if ((readCount % m_stepCount) == 0) {
      m_progress->setValue(readCount);
    }
    if (pd) {
        pd->setValue(readCount);
    }
    ep.processEvents();
    headword = m_query.value("word").toString();
    root = m_query.value("root").toString();
    node = m_query.value("nodeid").toString();
    page = m_query.value("page").toInt();
    Place p = Place::fromEntryRecord(m_query.record());
    if (headword.indexOf(rx) != -1) {
      if (options.includeHeads()) {
        int row = addRow(p,m_headText,0);
        headCount++;
      }
    }
    QString xml = m_query.value("xml").toString();
    QTextDocument * doc  = fetchDocument(xml);
    if (doc->characterCount() > 0) {
            getTextFragments(doc,target,options);
            if (m_fragments.size() > 0) {
              entryCount++;

              if (m_singleRow) {
                /// pass the first text fragments and the total number of fragments
                addRow(p,m_fragments[0],m_fragments.size());
              }
              else {
                for(int i=0;i < m_fragments.size();i++) {
                  if (m_fragments[i].size() > 0) {
                    /// pass the i'th fragment and the index of the fragment
                    addRow(p,m_fragments[i],i);
                  }
                }
              }
              textCount += m_fragments.size();
            }
    }

  }
  m_rxlist->setUpdatesEnabled(true);
  qint64 et = QDateTime::currentMSecsSinceEpoch();
  if (pd) {
    delete pd;
  }

  if (m_rxlist->rowCount() > 0) {
    m_container->removeItem(m_spacer);
    m_rxlist->show();
    m_rxlist->selectRow(0);
    m_rxlist->setFocus();
    m_exportButton->setEnabled(true);
  }
  else {
    m_exportButton->setEnabled(false);
  }
  m_resultsText->setText(buildText(entryCount,headCount,textCount,et - st));
  m_resultsText->show();
  this->show();
}
void FullSearchWidget::setForceLTR(bool v) {
   m_findTarget->setForceLTR(v);
 }
void FullSearchWidget::onExport() {
  statusMessage(m_rxlist->exportResults());
}
void FullSearchWidget::openNode(const QString & node) {
  emit(showNode(node,false));
  this->setFocus();
}
int FullSearchWidget::findCount() const {
  return m_findCount;
}
QString FullSearchWidget::results() const {
  return m_resultsText->text();
}
bool FullSearchWidget::startsWithArabic(const QString & t) const {
  for(int i=0;i < t.size();i++) {
    if (t.at(i).direction() == QChar::DirAL) {
      return true;
    }
    if (t.at(i).direction() == QChar::DirL) {
      return false;
    }

  }
  return false;
}
void FullSearchWidget::languageSwitch(int /* index */) {
  /*
  if (m_search->isArabicSearch()) {
    m_findTarget->enableMapping(m_mapEnabled);
  }
  else {
    m_findTarget->enableMapping(false);
  }
  */
}
