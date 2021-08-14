![kuvapaappaus_kajastin](https://user-images.githubusercontent.com/78755456/129454816-a6a3d250-4494-4fd0-8a45-00ad0ba920f2.png)![kuvakaappaus_kuutio](https://user-images.githubusercontent.com/78755456/129454818-45133102-9efb-48da-a9ec-9adf859bb51c.png)

Tämä vaatii grafiikkakirjastot SDL2 ja SDL_TTF. Tiedoston asetelma.c alkupuolella on muutama polku, jotka käyttäjän pitää asettaa itse.

Tässä on kuutiointiin tarkoitettu ajanottosovellus ja virtuaalinen rubikin kuutio. Nämä ovat kaksi itsenäistä C-ohjelmaa, mutta ne voivat myös keskustella keskenään ja ovat siksi samassa kansiossa. Pieniä lisätoimintoja on kirjoitettu myös Pythonilla.

Ajanottosovellus näyttää listan tuloksista sekä oikeassa että parhausjärjestyksessä; listan sekoituksista, jotka arvotaan joka kerralle; viiden ja kahdentoista keskiarvoista nykyisen, parhaan ja huonoimman sekä keskihajonnat; kaikista ajoista karsitun keskiarvon ja mediaanin. Parhausjärjestyksessä olevasta tuloslistasta näkyy alku- ja loppupää.

Painikkeella eri_sekunnit saa näkyviin listan, jonka neljä saraketta ovat "kokonaissekunti, tähän kokonaissekuntiin kuuluvien tulosten määrä, kyseisten tulosten osuus kaikista tuloksista, osuuden kertymä siihen asti". Liikuttamalla hiirtä tulos- tai järjestyslistan päällä näkee, millä hetkellä mikäkin tulos on saatu. Tuloksia voi myös tallentaa tiedostoon ja tuloksista voi piirtää kuvaajan painikkeella kuvaaja.

Napauttamalla hiirellä tuloslistalta haluttua tulosta leikepöydälle kopioituu kyseisen tuloksen tiedot: aika, sen saamisen ajankohta ja sekoitus kuten alla

11,32; sunnuntai 08.08.2021 klo 00.30  
L  D' U2 L2 D  U' F' U2 B2 U' D2 F  B  D  F  B' D2 U' L' R  

Napauttamalla viiden tai 12:n keskiarvoa hiiren oikealla painikkeella leikepöydälle kopioituu seuraavanlaiset tiedot kyseisestä keskiarvosta. Tässä keskihajonnan jakajana on n ja ei n-1, koska kyse on todellakin juuri näitten viiden tuloksen keskiarvosta, jolloin tämä ei ole mielestäni otos, vaan tässä ovat kaikki arvot ja siksi n-1 olisi mielestäni väärä jakaja.

Avg5: 15,34; σ = 0,69; sunnuntai 08.08.2021 klo 00.36  
(3. 14,23)   F2 L' B  F2 L  F  B2 D' U' F' R2 B' L2 R  B2 U2 F' U  F2 R'  
 4. 16,24   L2 D  U  L2 U2 R' U' F2 R  U  D  R2 L  B2 R' L' U2 F2 L' D'  
 5. 14,55   U  R' F' R' L  F  U2 D  B2 D' F2 B2 R2 U2 R  L2 B2 L2 B  L   
(6. 17,04)   D' B' R  L  D2 L  U2 B' U2 L2 D2 R2 L2 B' L2 F  U  D' L2 R2  
 7. 15,23   D  U' B  L  B' L2 R' D  U  R' L  D  R' L2 U  R  U2 D' R  B
 
Vasemmalla painikkeella sama tulostuu näytölle, mutta ilman sekoituksia.

Viimeisemmän tuloksen voi pyyhkiä korjausnäppäimellä (backspace). Minkä tahansa tuloksen voi poistaa painamalla ctrl ja napauttamalla hiirellä haluttua tuloslistan kohtaa. Tällöin sekoituksista poistetaan kyseiseen tulokseen kuuluva sekoitus. Korjausnäppäimellä sen sijaan poistuu viimeisin sekoitus, joka ei siis kuulu viimeisimpään tulokseen, vaan kuuluisi vasta seuraavalle tulokselle, jota ei vielä ole. Sekoituksia on siis tietenkin listalla aina yksi enemmän kuin tuloksia.

Viimeisemmän tuloksen rangaistuksen voi vaihtaa plus-näppäimellä ja minkä tahansa voi vaihtaa hiiren oikealla painikkeella halutusta tuloslistan kohdasta. Mahdollisia rangaistuksia ovat ei_mitään, +2 ja DNF, jotka vaihtuvat vuoron perään. Suuremmat rangaistukset kuten +4 sekuntia täytyy toistaiseksi toteuttaa syöttämällä manuaalisesti haluttu aika.

Ajanottosovelluksessa on seuraavat kirjoitustilat, joissa haluttu syöte kirjoitetaan näytölle ja painetaan Enter tai poistutaan ESC-näppäimellä:
- Ajan syöttö manuaalisesti, joka aukeaa napauttamalla ajanottoaluetta. DNF merkitään Ø-merkillä.
- Sekoituksen vaihtaminen (NxNxN)-kuutiolle. Tämä aukeaa napauttamalla sekoitusta hiiren oikealla painikkeella. Tähän syötetään luku N.
- Tuloslistan rullaaminen haluttuun kohtaan. Tämä aukeaa napauttamalla tuloslistaa hiiren keskipainikkeella.
- Keskiarvon karsinnan vaihtaminen. Keskiarvon perässä näkyy luku N (oletuksena 16), jota napauttamalla tämä kirjoitustila aukeaa. Keskiarvoa karsitaan aina jättämällä paras ja huonoin tulos pois. Aina kun tuloksia on N lisää, jätetään yksi paras ja huonoin tulos enemmän pois eli esim 20:tä tuloksesta jätettäisiin oletuksena kaksi parasta ja huonointa pois ennen keskiarvon laskemista. Tämä ei vaikuta 5:n ja 12:n keskiarvoihin.
- Tallennusnimen vaihtaminen, mikä aukeaa painamalla s-näppäintä tai hiirellä kohdasta "ulosnimi".

Ajanottosovelluksesta voi käynnistää kuution napauttamalla vasemmalta ylhäältä tekstiä kuutio. Kuution koko (NxNxN) on sama kuin mikä on asetettu sekoitukseen.
Näppäimellä F1 kuutio päätyy siihen sekoitukseen, jonka ajanottosovellus on arponut näytölle ja tarkasteluaika käynnistyy. Ajanotto alkaa automaattisesti, kun kuutiolle tekee siirron ja päättyy, kun kuutio on ratkaistu.

Kuutiota ohjataan näppäimistöltä tiedostossa kuutio.c määritetyin näppäimin. Näppäimet on valittu siten, että käyttäminen olisi helppoa kymmensormijärjestelmällä. Kuutiota voi myös pyöritellä hiirellä. Näppäimellä F2 kuutio avaa Python-sovelluksen, joka kuuntelee säveliä ja näin kuutiota voi ohjata myös viheltämällä, mutta tämä on lähinnä kokeellinen ominaisuus. Käynnistettäessä kuutio komentoriviltä sen koko on oletuksena 3 (3x3x3), mutta argumenttina voi antaa muun koon, esim. "./kuutio 6" käynnistäisi (6x6x6)-kuution.

Kuutiossa on myös jotain hiirellä ja nuolilla toimivia tahkojen tai ruutujen korostustoimintoja, jotka ovat turhia ja tehty lähinnä virheenjäljitystarkoituksessa.

Tulokset voi tallentaa ctrl+s-komennolla. Tämä tallentaa ajat ja niitten saamisajankohdat, mutta ei sekoituksia. Kirjoitettavan tiedoston nimi lukee kohdassa ulosnimi ja sitä voi vaihtaa yllä kuvatulla tavalla. Jos tiedosto on jo olemassa, kirjoitetaan sen loppuun, mutta jos on samaan aikaan saatuja tuloksia, kirjoitetaan niitten päälle, jottei kaksi peräkkäistä tallennusta tuottaisi samoja tuloksia kahdesti.

Aiemmin tallennettujen tulosten tarkastelu tällä sovellukeslla onnistuu antamalla tiedoston nimi komentoriviargumenttina: ./kajastin TIEDOSTON_NIMI. Tiedostosta voi myös lukea vain osan antamalla rajausargumentin komentorivillä. Esimerkiksi viimeiset tuhat tulosta rajataan komennolla "./kajastin tulokset.txt -1000:", tai ensimmäiset tuhat rajataan argumentilla ":1000". Tarkempi selitys on tiedostossa main.c.

Joittenkin alueitten fonttikokoa voi muuttaa ctrl+rullaus toiminnolla.
