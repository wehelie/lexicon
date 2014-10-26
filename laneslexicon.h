#ifndef __LANESLEXICON_H__
#define __LANESLEXICON_H__
#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidgetItem>
#include <QGridLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QToolBar>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QStatusBar>
#include <QDebug>
#include <QSplitter>
#include <QLineEdit>
#include <QTextEdit>
#include <QStackedLayout>
#include <QSplitter>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTreeView>
#include <QSqlDatabase>
#include <QFile>
#include <QIcon>
#include <QLabel>
#include <QDialog>
#include <QKeySequenceEdit>
#include <QSettings>
#include <QDateTime>
#include <QShortcut>
#include <QSignalMapper>
#include <QMessageBox>
#include <QDockWidget>
#include <QInputDialog>
#include <QActionGroup>

#include <QUrl>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include "place.h"
#include "searchoptionswidget.h"
#ifndef qStrip
#define qStrip qDebug()
#endif

class ArabicSearchDialog;
class NoteMaster;
class HistoryMaster;
class Place;
class ArabicSearchDialog;
class AppMenu;
class InputMapper;
class GraphicsEntry;
class ContentsWidget;
class TabWidget;
class EntryLayoutWidget;
class LogViewer;
class LanesLexicon : public QMainWindow
{
  Q_OBJECT

 public:
  explicit LanesLexicon(QWidget *parent = 0);
  ~LanesLexicon();

  QSize sizeHint() const;
  void readSettings();
  void writeSettings();
  bool isOk() { return m_ok;}
  QString convertString(const QString &) const;
  QString getActiveKeymap() const;
  HistoryMaster * history();
  NoteMaster * notes();
  public slots:
    void gotoPlace(const Place &,int);
    int hasPlace(const Place & p,int searchtype,bool setFocus);
    void setStatus(const QString &);
    //    void saveNote(Note *);
    private slots:
      void onLogViewer();
      void onLinkChanged();
      void testSlot();
      void sync();
      void showNoteBrowser();
      void shortcut(const QString &);
      void deleteSearch();
      void localSearch();
      void localSearchNext();
      void localSearchClear();
      void localSearchShow();
      void tabsChanged();

      void reloadEntry(const QString &,const QString &);
      void revertEntry();
      void findNextRoot(const QString &);
      void findPrevRoot(const QString &);

      void moveNext(const Place &);
      void movePrevious(const Place &);
      void onCloseTab(int) ;
      void onCloseOtherTabs() ;
      void onGoToPage(const Place &);
      void onExit();
      void onTest();


      void onNavigationMenuChanged(QAction *);       // change navigation by menu
      void onNavNext();
      void onNavPrev();
      void onNavFirst();
      void onNavLast();
      void onNextRoot();
      void onPrevRoot();
      void onFirstRoot();
      void onLastRoot();
      void onNextPage();
      void onPrevPage();
      void onFirstPage();
      void onLastPage();

      void searchForWord();
      void searchForPage();
      void searchForNode();
      void searchForRoot();
      void searchForEntry();

      void onClearHistory();
      void onHistorySelection();

      void onKeymapChanged();

      void onEditView();

      void onOptions();

      void onSetInterface(bool triggered=false);
      void rootClicked(QTreeWidgetItem * , int);
      void entryActivated(QTreeWidgetItem * , int);
      void focusItemChanged(QGraphicsItem *, QGraphicsItem *, Qt::FocusReason);

      void rootChanged(const QString & root,const QString & node);
      void placeChanged(const Place &);
      void historyPositionChanged(int);
      void historyAddition(const Place &);

      void bookmarkShortcut(const QString &);
      void bookmarkAdd();
      void bookmarkAdd(const QString & id,const Place & p);


      void onDocs();
      void onAbout();
      void bookmarkRebuildMenu();

      void currentTabChanged(int);
      /// these apply to the current graphicsentry
      void pageZoomIn();
      void pageZoomOut();
      void pageWiden();
      void pageNarrow();
      void pagePrint();
      void pageSearch();
      void pageClear();
      void pageSearchComplete();
      void syncContents();
      /// from FullSearchWidget
      void showSearchNode(const QString &);
      void convertToEntry();
      void printNode(const QString &);
 protected:
      void closeEvent(QCloseEvent *);
 private:
      void printCurrentPage(const QString & node = QString());
      void restoreSavedState();
      void cleanup();
      void search(int searchType,ArabicSearchDialog *,const QString &);
      Place showPlace(const Place &,int);
      void updateStatusBar();
      void updateMenu();
      int searchTabs(const QString & node);
      void restoreBookmarks();
      void setupBookmarkShortcuts();
      void bookmarkJump(const QString & id);
      void setupShortcuts();
      int m_historyPos;
      void bookmarkClear();
      void setSignals(GraphicsEntry *);
      void loadStyleSheet();
      void createActions();
      void setIcon(QAction *,const QString &,const QString &);
      void setIcons(const QString & theme = QString());
      void createToolBar();
      void setupHistory(int startPos = -1);
      void createMenus();
      void createStatusBar();

      void enableKeymaps(bool);
      bool m_keymapsEnabled;
      bool eventFilter(QObject * target, QEvent *);
      bool openDatabase(const QString &);
      void getFirstAndLast();
      void restoreTabs();
      Place getCurrentPlace();
      int getSearchCount();
      void addBookmarkMenuItem(const QString & id);

      //    QAction * createIconAction(const QString imgdir,const QString & iconfile,const QString & text);
      /// look through all tabs for the given node, -1 if not found, else tab index

      SearchOptions m_defaultSearchOptions;
      SearchOptions m_searchOptions;

      QString m_configFile;
      QString m_interface;    // "default","minimal"
      QString m_applicationCssFile;
      QString m_printPdfLocation;
      QString m_iconTheme;
      QString m_nullMap;
      QString m_currentMap;
      QString m_dbName;
      QString m_notesDbName;
      QString m_historyDbName;
      QString m_startupNode;
      QString m_startupRoot;
      QString m_firstRoot;
      QString m_lastRoot;

      QPrinter m_printer;

      int m_navMode;
      int m_firstPage;
      int m_lastPage;

      QLabel * m_placeIndicator;

      bool m_toolbarText;
      bool m_linkContents;
      bool m_printToPdf;
      bool m_printerReUse;
      bool m_ok;
      bool m_docked;          // whether to use docked widget
      bool m_valgrind;
      bool m_useNotes;
      bool m_historyEnabled;
      bool m_saveTabs;
      bool m_saveSettings;
      bool m_restoreTabs;
      bool m_searchNewTab;
      bool m_searchSwitchTab;
      bool m_interfaceWarning;
      bool m_treeKeepsFocus;
      bool m_saveBookmarks;
      bool m_restoreBookmarks;
      bool m_revertEnabled;

      QToolBar * m_mainbar;
      QToolBar * m_navigation;
      QToolBar * m_entrybar;

      QFont arFont;

      InputMapper * m_mapper;

      QStringList m_definedMaps;

      QSignalMapper * m_signalMapper;
      QSignalMapper * m_bookmarkMap;

      QMap<QString, Place> m_bookmarks;

      QMenu * m_bookmarkMenu;
      QMenu * m_historyMenu;
      QMenu * m_fileMenu;
      QMenu * m_searchMenu;
      QMenu * m_navigationModeMenu;
      QMenu * m_moveMenu;
      QMenu * m_viewMenu;
      QMenu * m_pageMenu;
      QMenu * m_toolMenu;
      QMenu * m_helpMenu;
      AppMenu * m_mainmenu;

      QSize m_toolbarIconSize;

      Place m_place;

      ContentsWidget * m_tree;
      TabWidget * m_tabs;
      QDockWidget * m_treeDock;
      EntryLayoutWidget * m_entryLayout;
      LogViewer * m_logview;



      QSqlDatabase m_db;
      QSqlDatabase m_notesDb;



      QToolButton * m_bookmarkButton;
      QToolButton * m_exitButton;
      QToolButton * m_searchButton;
      QToolButton * m_docButton;
      QToolButton * m_aboutButton;
      QToolButton * m_optionsButton;
      QToolButton * m_logButton;
      QToolButton * m_zoomInButton;
      QToolButton * m_zoomOutButton;
      QToolButton * m_widenButton;
      QToolButton * m_narrowButton;
      QToolButton * m_printButton;
      QToolButton * m_localSearchButton;
      QToolButton * m_localSearchNextButton;
      QToolButton * m_clearButton;
      QToolButton * m_navFirstButton;
      QToolButton * m_navLastButton;
      QToolButton * m_navNextButton;
      QToolButton * m_navPrevButton;
      QToolButton * m_navigationButton;
      QToolButton * m_historyButton;
      QToolButton * m_keymapsButton;
      QToolButton * m_linkButton;

      QActionGroup * m_moveGroup;



      QAction * m_minimalAction;
      QAction * m_logViewerAction;
      QAction * m_optionsAction;
      QAction * m_bookmarkListAction;
      QAction * m_bookmarkAddAction;
      QAction * m_bookmarkJumpAction;
      QAction * m_bookmarkClearAction;
      QAction * m_bookmarkRevertAction;
      QAction * m_docAction;
      QAction * m_exitAction;
      QAction * m_testAction;
      QAction * m_editViewAction;
      QAction * m_historyAction;
      QAction * m_bookmarkAction;
      QAction * m_searchAction;
      QAction * m_searchWordAction;
      QAction * m_searchPageAction;
      QAction * m_searchRootAction;
      QAction * m_searchNodeAction;
      QAction * m_searchEntryAction;
      QAction * m_navNextAction;
      QAction * m_navPrevAction;
      QAction * m_navFirstAction;
      QAction * m_navLastAction;
      QAction * m_navModeRootAction;
      QAction * m_navModePageAction;
      QAction * m_clearHistoryAction;
      QAction * m_zoomInAction;
      QAction * m_zoomOutAction;
      QAction * m_widenAction;
      QAction * m_narrowAction;
      QAction * m_printAction;
      QAction * m_localSearchAction;
      QAction * m_localSearchNextAction;
      QAction * m_clearAction;                     // clear the highlights
      QAction * m_convertToEntryAction;      // convert HeadSearch to GraphicsEntry
      QAction * m_keymapsAction;
      QAction * m_linkAction;
      QAction * m_aboutAction;

      HistoryMaster * m_history;
      NoteMaster * m_notes;


 signals:
      void nodeActivated(const QString & node,const QString & word);
};
#endif
