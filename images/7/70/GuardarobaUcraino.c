/*PREMESSA:
sia la wait() che la signal() sono BLOCCANTI nei monitor!!
La DIFFERENZA è che con la wait() il processo chiamante si sospende in attesa della signal() sulla condition di cui ha fatto la wait()
mentre quando si chiama la signal() il processo chiamante rimane in attesa nell'urgent stack fino a che il processo che aveva chiamato 
la wait(), e che grazie alla signal() si era sbloccato, non termina!!!

DIO SIA LODATO!!! HO VISTO LA LUCE!!! (spero)*/



Monitor Guardaroba{


const int MAXGIACCHE;   /* numero massimo di giacche */
int codaIN;				/*contatore persone coda ingresso */
int codaOUT;				/*contatore coda persone uscita  */
int giacche;				/*contatore giacche  */


union item { giacca , contrassegno };		/*Una variabile tipo item può assumere 1 solo valore tra GIACCA e CONTRASSEGNO*/

item oggConsegnato;		 /*Variabile di tipo item che serve al guardarobiere per decrementare o incrementare il contatore delle giacche*/

condition OKConsegna; 		/*(codaIN < codaOUT) && (giacche < MAXGIACCHE) oppure (codaIN > 1)*/
condition OKPass; 		/* oggConsegnato==GIACCA */
condition OKRitiro; 		/*(codaOUT < codaIN) || (giacche==MAXGIACCHE) oppure codaOUT>1*/

/**********************************************************************************************************/

procedure entry item prendi(){

while (oggConsegnato!=NULL){ 
};

if (oggConsegnato==GIACCA) giacche++;			/*SE oggConsegnato==GIACCA ==> incrementa il numero delle giacche presenti*/	/*1*/
else giacche--;	/*SE oggConsegnato==contrassegno ==> decrementa il numero delle giacche presenti*/

return oggConsegnato;	/*Ritorna, in ogni caso, l'oggConsegnato (Giacca / Contrassegno)*/
}

/**********************************************************************************************************/

procedure entry void dai(item y){
if (y==CONTRASSEGNO) codaOUT--; 	/*2*/	//Se y è un contrassegno ==> Decremento la coda dei visitatori in uscita perchè ne esce uno
if (y==GIACCA)  codaIN--;	//Se y è una giacca ==> Decremento la coda dei visitatore di entrata perchè ne entra uno
okPass.signal;	//Mi fermo e risveglio un processo che ha chiamato una okPass.wait(qui il processo che ha chiamato la signal() si mette in urgent Stack)
if(codaOUT!=0 && giacche < MAXGIACCHE)	{	//Controllo che la coda dei visitatori uscenti sia diversa da 0 perchè altrimenti la condizione dopo risulterebbe sempre falsa andando in deadlock e che ci sia ancora posto nel guardaroba
	if (codaOUT > codaIN)   //Se la coda di entrata è più corta di quella di uscita
		OKConsegna.signal;	//permetto a un altro di consegnare la giacca (e qui entro in attesa se qlc1 altro ha chiamato prima di me una OKConsegna.wait())
	else OKRitiro.signal;	//ALTRIMENTI permetto a un visitatore di ritirare la sua giacca
}

else
	OKConsegna.signal;	 //Se non ci sono visitatori in uscita che devono ritirare la giacca, permetto a uno in entrata di posare la sua nel guardaroba

oggConsegnato= NULL /* azzero l'oggetto consegnato */
}

/**********************************************************************************************************/

procedure entry contrassegno consegna(giacca g){
if( giacche == MAXGIACCHE )				/*Dobbiiamo controllare che il guardaroba non sia già pieno per non accettare giacche che non si possono posare! */
	OKConsegna.wait;	//Se non c'è più posto nel guardaroba devo attendere
else
	codaIN++;	/*La giacca può essere accettata ==> incremento il numero dei visitatori in attesa*/
	if (codaOUT !=0)		/*Se i visitatori in uscita sono 0, devo prendere le giacche di quelli in entrata.*/ 
		if ( (codaIN >= codaOUT) ) /*4*/ 	//Se la coda di ingresso è maggiore, o uguale(scelta progettuale), di quella di uscita,
			OKConsegna.wait();	//I Visitatori che vogliono consegnare la giacca devono attendere (qui il processo si blocca e finisce nella coda dei processi in attesa della condition OKConsegna)
	//Se in uscita ci sono più visitatori che in entrata ==> consegno la giacca assegnando il valore di g a oggConsegnato che sblocca il ciclo prendi() del guardarobiere
	oggConsegnato = g; 
	okPass.wait; //e attendo che mi venga consegnato il contrassegno (e qui il processo si blocca finendo nella coda dei processi in attesa della condition okPass  )
	
	contrassegno c = oggConsegnato.contrassegno;	//Memorizzo il contrassegno in c e ritorno il suo valore in uscita (che verrà usato dalla variabile c della procedure entry ritira)

	return c;
}
/**********************************************************************************************************/


procedure entry giacca ritira(contrassegno c){
if (c!=CONTRASSEGNO) OKRitiro.wait;	//Se non mi è stato passato un contrassegno attendo di riceverlo
/*ALTRIMENTI*/
codaOUT++; //La giacca può essere ritirata ==> Incremento la coda dei visitatori in uscita
if (codaIN !=0)	//Controllo che nella coda dei visitatori in entrata ci sia qualcuno per poter vedere quale coda servire prima
	if ( (codaOUT > codaIN) && (giacche<MAXGIACCHE) ) //Se la coda di entrata è minore di quella di uscita e ci sono ancora posti nel guardaroba
	OKRitiro.wait;		//I visitatori che devono ritirare la giacca attendono (e qui il processo si blocca in attesa di una signal da parte del guardarobiere)
/*ALTRIMENTI*/
oggConsegnato.contrassegno = c;	//Dò il contrassegno al guardarobiere (che sbloccherà il ciclo while della procedure entry prendi() del guardarobiere)
okPass.wait;	//Il visitatore si deve mettere in attesa della giacca che il guardarobiere gli va a prendere.

c = oggConsegnato.giacca; //Quando si sblocca il visitatore ha la giacca (in oggConsegnato) che restituisce in uscita e che verrà poi assegnata alla variabile di tipo ciacca g

return c;
}


/**********************************************************************************************************/