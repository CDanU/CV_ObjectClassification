# CV_ObjectClassification
## Required
### OpenCV v3.1.0: Open Source Computer Vision Library

* Homepage: <http://opencv.org>
* Docs: <http://docs.opencv.org/master/>
* Q&A forum: <http://answers.opencv.org>
* Issue tracking: <https://github.com/Itseez/opencv/issues>

## Todo (german)
Klassen finden:
* 1vsAll: unbekanntes Bild gegen alle Bilder in der DB
* Clusterzentren: dienen als Klassenrepräsentanten
* Decision Trees: Verzweigungsbaum anhand von FV

| % | Schwierigkeit | Auswahl Bereiche/Bildpunkte | Bestimmung der Features | Quantisierung | Histogramm-Bildung | Klassenentscheidung / Bewertung
|-
| 0% | easy | alle Pixel, Durschnittsfarbe |-|-|-| euklidischer Abstand|
| 0% | medium | zufällig, regelmäßiges Raster, Eckpunkte | Pixelwerte der Umgebung:Intensiätetn/Farbwerte, Kantenhistogram der Umgebung, SURF-Features | lineare Quantisierung |-| Diskriminative Verfahren, einfache Abstandsmaße, absoluter Abstand, Generative Verfahren
| 0% | hart  | SIFT-Keypoint-Detektion | SIFT-Features | Vektorquantisierung | Histogramm-Bildung |-
