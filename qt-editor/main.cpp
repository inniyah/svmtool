#include <QtGui>

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

		void setFormatFor(Construct construct,
			const QTextCharFormat &format);
		QTextCharFormat formatFor(Construct construct) const
			{ return m_formats[construct]; }

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
};

mySyntaxHighLighter::mySyntaxHighLighter(QTextDocument *document)
: QSyntaxHighlighter(document) {
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
