#include <qcustomplot.h>
#include <stdio.h>
#include <err.h>

struct Ikkuna: public QWidget {
    Ikkuna(QWidget* parent = NULL) {
	this->plot = new QCustomPlot;
    }
    ~Ikkuna() {
	delete this->plot;
    }
    void piirrä(int*, long*, int, int);
    QCustomPlot* plot;
};

void Ikkuna::piirrä(int* x, long* y, int pit, int n) {
    QVector<double> x1(pit), y1(pit);
    for(int i=0; i<pit; i++) {
	x1[i] = x[i];
	y1[i] = y[i];
    }
    auto ax = new QCPAxisRect(this->plot);
    this->plot->plotLayout()->addElement(n%2, n/2, ax);
    QCPGraph* graph = this->plot->addGraph(ax->axis(QCPAxis::atBottom), ax->axis(QCPAxis::atLeft));
    graph->setData(x1, y1);
    graph->setLineStyle(QCPGraph::lsNone);
    graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));
    graph->rescaleAxes();
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    Ikkuna ikk;
    ikk.plot->plotLayout()->clear();
    char nimi[] = "sarjat_.txt";

    for(char i='4'; i<='9'; i++) {
	nimi[sizeof(nimi)-6] = i;
	FILE *f = fopen(nimi, "r");
	if(!f) {
	    warn("fopen %s", nimi);
	    continue; }
	while(fgetc(f) != '\n');
	long alku = ftell(f);
	int pit = 0;
	while(!feof(f))
	    if(fgetc(f) == '\n')
		pit++;
	fseek(f, alku, SEEK_SET);
	int siirtoja[pit];
	long kpl[pit];
	for(int i=0; i<pit; i++) {
	    fscanf(f, "%i,%li,", siirtoja+i, kpl+i);
	    while(fgetc(f) != '\n');
	}
	fclose(f);
	ikk.piirrä(siirtoja, kpl, pit, i-'4');
    }
    ikk.plot->show();
    return app.exec();
}
