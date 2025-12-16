# IA_Project — Developer Function Manual

Purpose
- This manual summarizes the public and important internal functions in the main source files so you can quickly find where to make changes.
- Files covered are the primary application sources in `Development/Source_Code`.

Notes
- The application uses Qt Widgets. Many UI callbacks are connected either via explicit `connect(...)` calls or Qt's auto-connect feature (slot names like `on_<widgetName>_<signal>()`).
- The SQLite database file path is hardcoded in `MainWindow` constructor as `/home/brometheus/IA_Project/data/example.db` — change with care.

---

## `main.cpp`
- `int main(int argc, char *argv[])`
  - Entry point. Initializes `QApplication`, constructs `MainWindow`, shows it, and enters the event loop.

---

## `mainwindow.h` / `mainwindow.cpp`
- `MainWindow::MainWindow(QWidget *parent = nullptr)`
  - Constructor. Initializes UI, loads vocab lists (via `db.getVocabListsWithNextReview()`), sorts and populates `ui->deckList`, connects signals for deck interactions and buttons.
  - Instantiates `DataBase db` with the path above.
- `MainWindow::~MainWindow()`
  - Destructor that deletes the UI object.
- `DataBase* MainWindow::getDB()`
  - Returns pointer to the `DataBase` instance (useful for other dialogs/windows to access DB).
- `void MainWindow::on_addWord_clicked()`
  - Shows `AddCardWindow` modal dialog to add a new word/card.
- `void MainWindow::on_creatDeck_clicked()`
  - Shows `AddListWindow` modal dialog to create a new vocabulary list; connects its `newAddedList` signal to `updatingList()`.
- `void MainWindow::updatingList()`
  - Re-reads lists from DB and refreshes `ui->deckList` (same logic as constructor).
- `void MainWindow::startStudy()`
  - Starts study for the currently selected list in `ui->deckList` (reads list name from item text, gets list ID, constructs `StudyWindow`).
- `void MainWindow::deckListDoubleClicked(QListWidgetItem* item)`
  - Handler for double-click on deck list: stores pending selection, shows mode panel and hides deck list.
- `void MainWindow::on_startStudyButton_clicked()`
  - Invoked by UI: creates a `StudyWindow` using the pending list ID and sets study mode from UI combo box.
- `void MainWindow::on_listDecks_clicked()`
  - Top "Decks" button: shows the deck list panel and hides the mode panel.
- `void MainWindow::on_syncWords_clicked()`
  - New slot (connected automatically if `mainwindow.ui` has a `QPushButton` named `syncWords`). Calls `db.getStudySessionSummary()` and shows a modal `QDialog` containing the text summary.

Where to change behavior
- To adjust UI layout: edit `mainwindow.ui` in Qt Creator.
- To change list fetching/sorting: modify `getVocabListsWithNextReview()` in `database.cpp` or the sorting lambda in constructor/updatingList().

---

## `addcardwindow.h` / `addcardwindow.cpp`
- `AddCardWindow::AddCardWindow(QWidget *parent, DataBase* dataBase)`
  - Constructor. Sets up UI and populates `vocabListDropdown` from `db->getVocabLists()`.
- `AddCardWindow::~AddCardWindow()`
  - Destructor.
- `void AddCardWindow::on_cancelAdding_clicked()`
  - Slot for Cancel button: rejects the dialog.
- `void AddCardWindow::on_addButton_clicked()`
  - Gathers inputs (`listName`, `word`, `definition`, `partOfSpeech`), validates them, gets `listID` via `db->getListId`, calls `db->addWordAndSetup(...)` to add the word and initialize schedules. Also optionally adds examples via `db->createNewExample` if additional options checked.

Where to change behavior
- To change validation logic or fields gathered, edit `on_addButton_clicked()`.
- To adjust how new words are inserted into DB, modify `DataBase::addWordAndSetup` in `database.cpp`.

---

## `addlistwindow.h` / `addlistwindow.cpp`
- `AddListWindow::AddListWindow(QWidget *parent, DataBase* dataBase)`
  - Constructor. Sets up UI.
- `AddListWindow::~AddListWindow()`
  - Destructor.
- `void AddListWindow::on_cancelCreation_clicked()`
  - Rejects the dialog.
- `void AddListWindow::on_createList_clicked()`
  - Calls `db->createNewList(listName, language, description)`, emits `newAddedList()` signal so `MainWindow` refreshes lists, and closes the dialog.

Where to change behavior
- To modify list creation validations or extra metadata fields, edit `on_createList_clicked()`.

---

## `studywindow.h` / `studywindow.cpp`
- `StudyWindow::StudyWindow(DataBase* database, int listID_, QWidget* parent)`
  - Constructor. Sets UI styles, wires button and combo signal handlers, sets default study mode, loads due cards from DB (`loadDueCards()`), and displays the first card (`showCurrentCard()`).
- `void StudyWindow::loadDueCards()`
  - Calls `db->getDueCards(listID)` to get cards due for review and resets `currentIndex`.
- `void StudyWindow::showCurrentCard()`
  - Displays the current card. Branches by `mode`:
    - Flashcard mode: shows word and hides definition until `onReveal()`.
    - Multiple choice: prepares distractor choices via `db->getRandomWordsInList()` and shuffles them, sets up choice buttons.
  - If no more cards, informs and closes the window.
- `void StudyWindow::onReveal()`
  - Shows the definition for flashcards and disables the reveal button.
- `void StudyWindow::applyRating(int quality)`
  - Core: prepares a `SpacedRepetitionCalculator` with the card's stored schedule, calls `calculateNextReview(quality)`, formats the next review date string, updates scheduling in DB via `db->updateReviewScheduleForWord(...)`, records the study session with `db->recordStudySession(...)`, advances to the next card and shows it.
- `void StudyWindow::onRateAgain()` / `onRateHard()` / `onRateGood()` / `onRateEasy()`
  - Quick slots that call `applyRating()` with appropriate quality values (0/3/4/5).
- `void StudyWindow::onModeChanged(int index)`
  - Switches `mode` between `Flashcard` and `MultipleChoice` and refreshes display.
- `void StudyWindow::onChoiceSelected()`
  - Handler for multiple-choice buttons: determines if the chosen answer is correct, shows a message box, and calls `applyRating()` with appropriate quality.
- `void StudyWindow::setStudyModeFromString(const QString& modeStr)`
  - Helper for externally setting the study mode before showing the dialog.

Where to change behavior
- To change how quality maps to scheduling changes, edit `applyRating()` and/or `SpacedRepetitionCalculator::calculateNextReview`.
- To change distractor selection, modify calls to `db->getRandomWordsInList()` or its logic in `database.cpp`.

---

## `spacedrepetitioncalculator.h` / `spacedrepetitioncalculator.cpp`
- `SpacedRepetitionCalculator::SpacedRepetitionCalculator()`
  - Constructor. Initializes `easinessFactor` to 2.5, `repetitions` to 0, `interval` to 0, and `nextReview` to current time.
- `void SpacedRepetitionCalculator::calculateNextReview(int quality)`
  - Implements the SM-2-like easiness factor update formula. Clamps `quality` to [0..5], updates `easinessFactor` using formula, clamps `easinessFactor` to [1.3..2.5], updates `repetitions` and `interval` according to SM-2 rules, and computes `nextReview` as now + interval days.
- `bool isDue() const` (declaration present) — check implementation if needed.
- Getters: `getEasinessFactor()`, `getRepetitions()`, `getInterval()`, `getNextReview()`.
- Setters: `setEasinessFactor(double)`, `setRepetitions(int)`, `setInterval(int)`, `setNextReview(time_t)`. The setter for easiness factor clamps its value to the DB-allowed range.

Where to change behavior
- To change the scheduling algorithm (e.g., different formulas or minimum/maximum bounds), edit `calculateNextReview` and possibly the constants `MIN_EF` / `MAX_EF` in the header.

---

## `database.h` / `database.cpp`
- Constructor/Destructor
  - `DataBase::DataBase(const std::string& dbPath)` — opens the sqlite DB, enables foreign keys, and ensures tables are created by calling various `create*Table()` methods.
  - `DataBase::~DataBase()` — closes SQLite handle.
- PRAGMA / schema methods
  - `void enableForeignKeys()` — turns on `PRAGMA foreign_keys = ON;`.
  - `bool createVocabListTable()`, `bool createWordsTable()`, `bool createListWordTable()`, `bool createStudySessionTable()`, `bool createReviewScheduleTable()`, `bool createExampleTable()`, `bool createListProgressTable()`, `bool createWordRelationTable()` — create schema tables and indexes. Edit here if you need to change schema. Note: `createStudySessionTable()` defines a CHECK constraint for `confidence_score` (1-5) and `study_mode` allowed values; `createReviewScheduleTable()` defines an `ease_factor` CHECK between 1.3 and 2.5.
- Transaction helpers
  - `bool beginTransaction()`, `bool commitTransaction()`, `bool rollbackTransaction()` — convenience wrappers for sqlite transaction control.
- CRUD and helpers
  - `int getWordId(const std::string& word, const std::string& language)` — returns `word_id` if present.
  - `int getListId(const std::string& listName)` — returns `list_id`.
  - `int addOrGetWord(...)` — inserts a word if missing, returns the `word_id`.
  - `bool addWordToList(int listID, int wordID)` — create membership row.
  - `bool initReviewSchedule(int wordID, int listID)` — inserts initial review schedule row with defaults.
  - `bool incrementListProgress(int listID, int delta = 1)` — increments progress counters for a list.
  - `int addWordAndSetup(int listID, const std::string& word, ...)` — high-level convenience: add word, link to list, init schedule. Returns `word_id` or -1 on error.
  - `bool createNewList(std::string listName, std::string targetLanguage, std::string description)` — inserts a new vocabulary list.
  - `std::vector<std::string> getVocabLists()` — returns list of list names.
  - `std::vector<std::pair<std::string,std::string>> getVocabListsWithNextReview()` — returns list name and next review date string (used by `MainWindow`).
- Review schedule / cards
  - `struct DueCard` — used to pass schedule/card info (schedule_id, word_id, list_id, word, definition, ease_factor, interval_days, repetition_count, next_review_date).
  - `std::vector<DueCard> getDueCards(int listID = -1)` — returns cards whose `next_review_date <= now` (listID < 0 means all lists).
  - `bool updateReviewScheduleForWord(int wordID, int listID, int repetition_count, int interval_days, double ease_factor, const std::string& next_review_date)` — updates the `review_schedule` row for a word in a list. Note: this function now clamps `ease_factor` before binding to database to avoid violating the `CHECK` constraint.
- Study session recording
  - `bool recordStudySession(int wordID, int listID, bool was_correct, int quality)` — convenience overload that uses a default study mode (flashcard).
  - `bool recordStudySession(int wordID, int listID, bool was_correct, int quality, const std::string& study_mode)` — inserts a row into `study_sessions`. Clamps `confidence_score` to the allowed range before inserting.
  - `std::string getStudySessionSummary()` — (added) aggregates statistics: total sessions, correct count, percent correct, average response time, average confidence, and sessions grouped by `study_mode`. Returns a multi-line human-readable string.
- Other helpers
  - `std::vector<std::pair<int,std::string>> getRandomWordsInList(int listID, int excludeWordID, int count)` — used for multiple-choice distractors.
  - `bool createNewExample(int wordID, std::string exampleText, std::string contextNotes)` — inserts example sentences.
  - `bool createNewRelation(int word1ID, int word2ID, std::string relationType)` — inserts relations between words.

Where to change behavior
- To change how schedules are initialized, modify `initReviewSchedule` and/or the `INSERT OR IGNORE` defaults used when a word is added.
- To adjust database constraints (e.g., widen `ease_factor` range), edit the `CREATE TABLE` statement in `createReviewScheduleTable()` and any clamps in code that enforce them.
- To add extra fields to `study_sessions`, update `createStudySessionTable()` and `recordStudySession()`.

---

## UI files (`*.ui`)
The `.ui` files define widget hierarchy and widget names referenced in code. Key widgets used by code:
- `mainwindow.ui`
  - `syncWords` (`QPushButton`) — auto-connected to `MainWindow::on_syncWords_clicked()`.
  - `deckList` (`QListWidget`) — populated with lists; double-click handled by `deckListDoubleClicked`.
  - `startStudyButton` (`QPushButton`) — wired to `on_startStudyButton_clicked()`.
  - `modePanel`, `selectedDeckLabel`, `studyModeComboBox` — used for mode selection before starting study.
- `studywindow.ui`
  - `wordLabel`, `definitionLabel`, `revealButton`, rating buttons (`againButton`, `hardButton`, `goodButton`, `easyButton`), `choiceButton1..4` and `modeComboBox`.
- `addcardwindow.ui` / `addlistwindow.ui`
  - Inputs and buttons whose `on_*` auto-connect slots are implemented in the respective `.cpp` files.

Where to change UI
- Open `.ui` files in Qt Creator to edit layout or change widget object names. If you rename widgets, update corresponding `on_<widget>_<signal>` slots or explicit `connect()` calls.

---

## Third-party / generated files
- `sqlite3.c` / `sqlite3.h` — embedded SQLite; treat as a vendor dependency. Avoid changing unless you know SQLite internals.
- Files in `build/` are generated (moc files, ui headers). Do not edit manually.

---

## Common development tasks and where to implement them
- Add logging for DB errors: update `catch` blocks (e.g., in `studywindow.cpp`) to log more context before surfacing an error.
- Change scheduling algorithm: edit `SpacedRepetitionCalculator::calculateNextReview` and adjust DB clamps if algorithm produces wider ranges.
- Add export of study sessions: implement a new method in `DataBase` that queries `study_sessions` and either returns CSV text or writes to a file; add a UI button to trigger it (e.g., reuse `syncWords` or add a new button).
- Change the DB file location: update the hardcoded path in `MainWindow` constructor or change `DataBase` to accept a path from configuration/CLI.

---

If you'd like, I can:
- Generate a more detailed manual that includes exact SQL used by each DB method (copying SQL text), or
- Generate a per-line cross-reference mapping widgets -> slot names for all `.ui` files.

Tell me which extra detail you'd like added and I will update this file accordingly.
