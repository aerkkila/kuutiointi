tar -cvzf skello-202101.1.tar.gz skello.c grafiikka.c tulokset.c asetelma.c listat.c asetelma.h grafiikka.h listat.h tulokset.h muistin_jako.h Makefile asetelma.sh *.bmp kellonajat.c kellonajat.py kuvaaja.py kuutio.d/kuutio*.[ch] kuutio.d/lue_siirrot.c kuutio.d/main.c kuutio.d/python_savel.c kuutio.d/sävel.py kuutio.d/Makefile
kansio='/home/aerk/ohjelmat/skello'
rm -rf $kansio
mkdir $kansio
mv skello-202101.1.tar.gz $kansio
cp PKGBUILD $kansio
cd $kansio
sed -i 's/sha256sums.*/'"$(makepkg -g)"'/' PKGBUILD
