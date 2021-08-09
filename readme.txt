Grigoras Adrian, 322CC
Tema 2 - Protocoale de comunicatii

-------------------------------------------------------


1) server.c

Pentru a nu limita numarul de clienti sau numarul de topicuri, am implementat o lista simplu inlantuita pentru a retine fiecare topic, iar in topic am o lista de subscriberi care contin campuri ca: socket, sf si online_status. Atunci cand un client se deconecteaza i se schimba online_status in 0 in lista de topicuri si la fel si in lista de clienti logati. Apoi am scris niste functii pentru liste.

Atat pentru server.c si client.c m-am folosit de skel-ul de la laboratorul 8.
Am creat socketul pentru listen pentru a comunica cu clientul tcp si cu clientul udp(ca in laboratorul 6)
Am setat file descriptorul atat pentru socketul de comunicat cu tcp cat si pentru cel cu udp.
Apoi intr-o bucla infinita vad daca se asculta in momentul asta pe un anumit socket.
Daca gasesc verific de unde primesc:
Daca este socketul de udp:
	primesc mesajele cu recvfrom() si incep sa formez mesajul pentru a-l trimite clientului(adaug ip-ul si portul)
	parcurg topicurile la care am abonati si verific lista de abonati a topicului pe care s-a trimis.
	daca online_status-ul clientului este 1, inseamna ca este online si pot trimite pachetul
	daca online_status-ul este 0, dar sf e 1, salvez mesajele intr-un camp al clientului special pentru aceste mesaje trimise cat este offline.
	pentru a nu concatena mesajele, un protocol la care m-am gandit ar fi sa trimit un sir "ACK" dupa fiecare pachet primit, format si afisat din client.c. Daca "ACK" a fost primit de catre server pot trece la urmatorul client la care trebuie sa trimit si implicit, la urmatorul pachet pe care trebuie sa il trimit.

Daca este socketul inactiv(se vrea o conexiune noua):
	accept conexiunea cu noul socket
	cu recv() primesc id-ul din client.c
	apoi verific urmatoarele:
		Daca e deja logat din trecut si e si online, e deja conectat(Already connected)
		Daca a fost logat si acum e iar online, il loghez si schimb socketul(New client)
		Iar daca nu a mai fost logat, il adaug pur si simplu(New client)

Daca s-au primit date de la un client tcp
	iau bufferul trimis de catre client
	daca numarul de caractere primite este 0, atunci conexiunea se inchide si trec online_status in structuri ca 0
	daca numarul de caractere e > 0:
		verific daca clientul isi da subscribe (il adaug in listele de topic)
		verific daca isi da unsubscribe (il sterg din listele de topic)
	

2) client.c

fac structura de socket cu ajutorul argumentelor.
setez optiunile file_descriptorului.
Si apoi intr-o bucla infinita am urmatoarele posibilitati
	Se scrie ceva de la tastatura:
		daca este exit, inchid clientul
		daca este subscribe, afisez mesajul ca si-a dat subscribe la un topic
		daca este unsubscribe, afisez ca a dat unsubscribe la un topic
	Se citeste de pe socket (mesajele trimise de server)
		primesc mesajul trimis de server
		apoi incep prelucrarea lui pentru afisare (scot ip-ul si port-ul)
		in functie de tipul sau(int, short, float sau string), scot din buffer informatiile necesare (se afla la pozitii diferite in functie de mesaj)
		apoi afisez mesajul si trimit acel "ack" pentru a continua comunicarea dintre server si client


	


