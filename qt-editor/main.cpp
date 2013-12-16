#include <QtGui>

#include <svmtool.h>
#include <cstdio>
#include <cstdlib>

#define SVMTOOL_MODEL_FILE "../models/eng/WSJTP"

class mySyntaxHighLighter: public QSyntaxHighlighter
{
	public:
		enum Construct
		{
			Entity,
			Tag,
			Comment,
			LastConstruct = Comment
		};

		mySyntaxHighLighter(QTextDocument *document);
		virtual ~mySyntaxHighLighter();

		void setFormatFor(Construct construct,
			const QTextCharFormat &format);
		QTextCharFormat formatFor(Construct construct) const
			{ return m_formats[construct]; }

		void asHtml(QString & html);

	protected:
		enum State
		{
			NormalState = -1,
			InComment,
			InTag
		};

		void highlightBlock(const QString &text);

	private:
		QTextCharFormat m_formats[LastConstruct + 1];
		SVMTool::tagger * m_tagger;

};

mySyntaxHighLighter::mySyntaxHighLighter(QTextDocument *document)
: QSyntaxHighlighter(document), m_tagger(NULL) {
	QTextCharFormat entityFormat;
	entityFormat.setForeground(QColor(0, 128, 0));
	entityFormat.setFontWeight(QFont::Bold);
	setFormatFor(Entity, entityFormat);

	QTextCharFormat tagFormat;
	tagFormat.setForeground(QColor(192, 16, 112));
	tagFormat.setFontWeight(QFont::Bold);
	setFormatFor(Tag, tagFormat);

	QTextCharFormat commentFormat;
	commentFormat.setForeground(QColor(128, 10, 74));
	commentFormat.setFontItalic(true);
	setFormatFor(Comment, commentFormat);

	m_tagger = SVMTool::CreateTagger(SVMTOOL_MODEL_FILE);
	SVMTool::InitializeTagger(m_tagger, 0, "LR", 7, 3, 0, 0);
}

mySyntaxHighLighter::~mySyntaxHighLighter() {
	SVMTool::DestroyTagger(m_tagger);
	m_tagger = NULL;
}

void mySyntaxHighLighter::setFormatFor(Construct construct, const QTextCharFormat &format) {
	m_formats[construct] = format;
	rehighlight();
}


void mySyntaxHighLighter::highlightBlock(const QString &text) {
	int state = previousBlockState();
	int len = text.length();
	int start = 0;
	int pos = 0;

	SVMTool::Result *ret = SVMTool::RunTagger(m_tagger, "Hello world ! It is a sample program .",9);
	//ret->print();
	delete ret;

	while (pos < len) {
		switch (state) {
			case NormalState:
			default:
				while (pos < len) {
					QChar ch = text.at(pos);
					if (ch == '<') {
						if (text.mid(pos, 4) == "<!--") {
							state = InComment;
						}
						else {
							state = InTag;
						}
						break;
					}
					else if (ch == '&') {
						start = pos;
						while (pos < len
							&& text.at(pos++) != ';')
							;
						setFormat(start, pos - start,
							m_formats[Entity]);
					}
					else {
						++pos;
					}
				}
				break;

			case InComment:
				start = pos;
				while (pos < len) {
					if (text.mid(pos, 3) == "-->") {
						pos += 3;
						state = NormalState;
						break;
					}
					else {
						++pos;
					}
				}
				setFormat(start, pos - start,
					m_formats[Comment]);
				break;

			case InTag:
				QChar quote = QChar::Null;
				start = pos;
				while (pos < len) {
					QChar ch = text.at(pos);
					if (quote.isNull()) {
						if (ch == '\"' || ch == '"') {
							quote = ch;
						}
						else if (ch == '>') {
							++pos;
							state = NormalState;
							break;
						}
					}
					else if (ch == quote) {
						quote = QChar::Null;
					}
					++pos;
				}
				setFormat(start, pos - start, m_formats[Tag]);
		}
	}

	setCurrentBlockState(state);
}


// See http://stackoverflow.com/questions/15280452/how-can-i-get-highlighted-text-from-a-qsyntaxhighlighter-into-an-html-string
void mySyntaxHighLighter::asHtml(QString & html) {
	// Create a new document from all the selected text document.
	QTextCursor cursor(document());
	cursor.select(QTextCursor::Document);
	QTextDocument* tempDocument(new QTextDocument);
	Q_ASSERT(tempDocument);
	QTextCursor tempCursor(tempDocument);

	tempCursor.insertFragment(cursor.selection());
	tempCursor.select(QTextCursor::Document);
	// Set the default foreground for the inserted characters.
	QTextCharFormat textfmt = tempCursor.charFormat();
	textfmt.setForeground(Qt::gray);
	tempCursor.setCharFormat(textfmt);

	// Apply the additional formats set by the syntax highlighter
	QTextBlock start = document()->findBlock(cursor.selectionStart());
	QTextBlock end = document()->findBlock(cursor.selectionEnd());
	end = end.next();
	const int selectionStart = cursor.selectionStart();
	const int endOfDocument = tempDocument->characterCount() - 1;
	for(QTextBlock current = start; current.isValid() and current not_eq end; current = current.next()) {
		const QTextLayout* layout(current.layout());

		foreach(const QTextLayout::FormatRange &range, layout->additionalFormats()) {
			const int start = current.position() + range.start - selectionStart;
			const int end = start + range.length;
			if(end <= 0 or start >= endOfDocument)
				continue;
			tempCursor.setPosition(qMax(start, 0));
			tempCursor.setPosition(qMin(end, endOfDocument), QTextCursor::KeepAnchor);
			tempCursor.setCharFormat(range.format);
		}
	}

	// Reset the user states since they are not interesting
	for(QTextBlock block = tempDocument->begin(); block.isValid(); block = block.next())
		block.setUserState(-1);

	// Make sure the text appears pre-formatted, and set the background we want.
	tempCursor.select(QTextCursor::Document);
	QTextBlockFormat blockFormat = tempCursor.blockFormat();
	blockFormat.setNonBreakableLines(true);
	blockFormat.setBackground(Qt::black);
	tempCursor.setBlockFormat(blockFormat);

	// Finally retreive the syntax higlighted and formatted html.
	html = tempCursor.selection().toHtml();
	delete tempDocument;
}


int main(int argc, char **argv) {
	QApplication app(argc, argv);

	QMainWindow *window = new QMainWindow();

	window->setWindowTitle(QString::fromUtf8("Test Editor"));
	window->resize(300, 250);

	QWidget *centralWidget = new QWidget(window);
	QHBoxLayout* layout = new QHBoxLayout(centralWidget);

	QTextEdit* edit = new QTextEdit();
	layout->addWidget(edit);

	mySyntaxHighLighter* highlighter = new mySyntaxHighLighter(edit->document());

	window->setCentralWidget(centralWidget);
	window->show();
	return app.exec();
}
