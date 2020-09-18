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
    <li>USB (UART4)</li>
    <li>Bluetooth (UART3)<br>
      <dd>Non riuscendo a utilizzare il modulo bluetooth integrato, per mancanza di documentazione, ci siamo affidati a una board esterna HM-10
    </li>
  </ul>
<h3>Funzionamento</h3>
  <ul>
    <p>Il sistema una volta alimentato crea:
      <ul>
        <li>La struttura dati per le letture dei diversi pannelli;</li>
        <li>Gli interrupt handler che si occupano di gestire gli interrupt relativi all'aggiornamento delle soglie (pulsante blu, carattere 'R' via bluetooth);
        <li>I mutex:
          <ul>
            <li>MUTEX 1 GABRY </li>
            <li>MUTEX 2 GABRY </li>
          </ul>
        <li>I thread:
          <ul>
            <li>Lettura pannello destro
              <ul>
                <p>Questo thread legge il valore analogico in output dal pannello destro (0.0 - 5.0) lo converte in una percentuale, per poi aggiornare in mutuale esclusione 
                   il relativo valore contenuto all'interno della struttura dati.
                </p>
              </ul>
            <li>Lettura pannello sinistro
              <ul>
                <p>Questo thread legge il valore analogico in output dal pannello sinistro (0.0 - 5.0) lo converte in una percentuale, per poi aggiornare in mutuale esclusione 
                   il relativo valore contenuto all'interno della struttura dati.
                </p>
              </ul>              
            <li>Stampe seriali
              <ul>
                <p>Questo thread in base alle macro settate stampa i valori relativi ai pannelli e alle soglie attuali su seriale (USB e/o Bluetooth).
                </p>
              </ul>
            <li>Gestione eventi di interrupt
              <ul>
                <p>Questo thread controlla il valore del flag di avvenuto interrupt (quelli precedentemente descritti), aggiorna i valori relativi alle soglie e lo resetta.<br>
                   Le soglie finora descritte riguardano:
                   <ul>
                     <p>Soglia minima, attuale valore più basso tra quelli registrati.</p>
                     <p>Variazione, differenza tra i valori attuali dei pannelli.</p>
                   </ul>
                </p>
              </ul>
            <li>Accensione led verde
              <ul>
                <p>L'accensione di questo led dichiara che i pannelli si trovano in una posizione di illuminazione ottimale, relativamente alle soglie attuali.<br>
                   Perchè si accenda il led bisogna che:<br>
                   <b>|Pannello DX - Pannello SX| < Variazione AND Pannello DX > Soglia minima AND Pannello SX > Soglia minima</b>
                 </p>
              </ul>
            <li>Accensione led blu
              <ul>
                <p>L'accensione di questo led dichiara che i pannelli non si trovano in una posizione di illuminazione ottimale è quindi necessario ruotarli in senso orario.
                   <br>
                   Perchè si accenda il led e si spenga il verde bisogna che:<br>
                   <b>|Pannello DX - Pannello SX| > Variazione AND Pannello DX >= Pannello SX</b>
                 </p>
              </ul>
            <li>Accensione led giallo
              <ul>
                <p>L'accensione di questo led dichiara che i pannelli non si trovano in una posizione di illuminazione ottimale è quindi necessario ruotarli in senso antiorario.
                   <br>
                   Perchè si accenda il led e si spenga il verde bisogna che:<br>
                   <b>|Pannello DX - Pannello SX| > Variazione AND Pannello DX < Pannello SX</b>
                 </p>
              </ul>
           </ul>
        <p>I thread una volta creati restano attivi operando nel rispetto dell'ordine seguente definito dai semafori.</p>
        <p><img alt="Image" title="icon" src="GABRY INSERISCI QUI L'IMMAGINE DELLA PARALLEL REGION.png" width="600" height="600" align="center"/></p>
    </p>
  </ul>
   
