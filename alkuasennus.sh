mkdir -p /usr/share/fonts/truetype
curl https://www.1001fonts.com/download/source-code-pro.zip -o font.zip
unzip font.zip
rm font.zip
sudo mv SourceCodePro**/*.ttf /usr/share/fonts/truetype/
rm -r SourceCodePro*

curl https://www.freefonts.io/wp-content/uploads/2021/05/verdana-font-family.zip -o ver.zip
unzip ver.zip
rm ver.zip
sudo mv verdana-font-family/verdana.ttf /usr/share/fonts/truetype
rm -r verdana-font-family

curl https://www.freefonts.io/wp-content/uploads/2021/10/dharma-type-sometype-mono.zip -o sometype.zip
unzip sometype.zip
rm *.otf
sudo mv Alternative\ files/*Regular.ttf /usr/share/fonts/truetype/SometypeMonoRegular.ttf
rm -r Alternative\ files
rm sometype.zip
