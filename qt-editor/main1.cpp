#include <QtGui>

class mySyntaxHighLighter: public QSyntaxHighlighter {

  public:    
	mySyntaxHighLighter(QTextDocument* document):
	QSyntaxHighlighter(document) {
	}

	~mySyntaxHighLighter() {
	}

	void highlightBlock(const QString &text) {
		enum { NormalState = -1, CStyleComment };

		int state = previousBlockState();
		int start = 0;

		for (int i = 0; i < text.length(); ++i) {
			if (state == CStyleComment){
				if (text.mid(i, 2) == "*/") {
					state = NormalState;
					setFormat(start, i - start + 2, Qt::blue);
				}
			} else {
				if (text.mid(i, 2) == "//") {
					setFormat(i, text.length() - i, Qt::red);
					break;
				} else if (text.mid(i, 2) == "/*"){
					start = i;
					state = CStyleComment;
				}
			}
		}
	};
};

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	QMainWindow *window = new QMainWindow();

	window->setWindowTitle(QString::fromUtf8("QTextEdit Highlight Comment Text"));
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
