<h1> Girasole Fotovoltaico </h1>
<p> Progetto Esame Sistemi Embedded e Real-time </p>
<p><img alt="Image" title="icon" src="G_IMG.png" width="600" height="600" align="center"/></p>

<h3> Studenti: </h3>
<ul>
  <li>Gabriele Felici</li>
  <li>Francesco Ghinelli</li>
</ul>

<h3>Intro</h3>
<ul>
  <p>In questo progetto abbiamo realizzato un girasole fotovoltaico che fa uso di due pannelli fotovoltaici per indicare la posizione di massima esposizione al sole.<br>
     Abbiamo anche pensato di utilizzare un motore per far si che i pannelli ruotassero alla ricerca della posizione ottimale ma ciò avrebbe richiesto lo sviluppo di un circuito        di potenza per la gestione di tale motore, il che usciva dagli obbiettivi di progetto; per questo motivo ci siamo limitati a indicare la direzione di rotazione mediante i led      presenti sulla board.
  </p>
</ul>

<h3>Input</h3>
  <ul>
    <li>Pannello Fotovoltaico Destro<br>
        <dd>Input analogico (Pin A4 + GND) che sfrutta il convertitore analogico-digitale ADC2 per leggere il valore.</dd><br>
    </li>
    <li>Pannello Fotovoltaico Sinistro<br>
        <dd>Input analogico (Pin A5 + GND) che sfrutta il convertitore analogico-digitale ADC1 per leggere il valore.</dd><br>
    </li>
    <li>Pulsante blu<br>
       <dd>Input digitale</dd>
  </ul>
<h3>Output</h3>
  <ul>
    <li>Led Verde</li>
    <li>Led Blu-Giallo</li>
  </ul>
<h3>Comunicazione seriale</h3>
  <ul>
    <p>Utilizzata per leggere i valori percentuali dei pannelli, le soglie attuali e per aggiornare i valori di soglia</p>
    <li>USB</li>
    <li>Bluetooth</li>
  </ul>
<h3>Funzionamento</h3>
  
