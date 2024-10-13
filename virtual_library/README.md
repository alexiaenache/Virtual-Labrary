Enache Alexia  
321CA  
# TEMA 4 PCOM - CLIENT WEB #

## Organizarea temei ##

- Am inceput de la codul din laboratorul 9. Am modificat putin comenzile de POST si GET, ca sa pot sa adaug tokenul JWT in header cand este cazul. In plus, am mai adaugat request-ul de tip DELETE, folosit pentru delete_book. A fost asemanator cu GET, singura diferenta fiind numele requestului.
- In helpers am adaugat cateva functii pentru prelucrarea datelor primite de la stdin si server.
- Fisierul commands contine toate comenzile care pot fi introduse de catre client. Sunt explicate in sectiunea urmatoare.
- Logica principala se afla in client.cpp
- Am folosit `parson` pentru parsare JSON, pentru ca mi s-a parut usor de utilizat, si am gasit tot ce aveam nevoie in [README](https://github.com/kgabis/parson/blob/master/README.md).

## Detalii implementare ##

- In momentul in care porneste clientul, acesta nu are cookie sau token JWT. Le poate obtine dupa ce se autentifica si obtine acces la biblioteca.
- Comenzile acceptate sunt:
  - **register**. Prin aceasta comanda clientul poate crea un cont nou. Daca username-ul este deja folosit, server-ul va intoarce un mesaj de eroare, altfel va fi creat contul.
  - **login**. Credentialele sunt trimise catre server. Daca sunt valide, se intoarce un cookie, altfel este afisat un mesaj de eroare. (**daca aveam deja un cookie/token jwt inainte de aceasta comanda, sunt sterse si dupa caz, inlocuite de cele noi**)
  - **enter_library**. Daca clientul este logat, comanda va intoarce un token JWT, prin care se obtine acces la biblioteca. Altfel, va intoarce un mesaj de eroare.
  - **get_books**. Daca clientul are acces la biblioteca, in urma comenzii, primeste toate cartile pe care le are.
  - **get_book**. Dupa ce se citeste si valideaza id-ul, clientul primeste cartea cu id ul cerut, sau eroare daca aceasta nu exista
  - **delete_book**. A fost asemanator cu get_book, numai ca se trimite un request DELETE. Daca cartea exista, se sterge, altfel se primeste un mesaj de eroare.
  - **logout**. Se sterg tokenul JWT si cookie-ul, daca clientul este autentificat.
  - **exit**. Se opreste programul
  - pentru orice alta comanda, se va afisa mesajul **ERROR: Invalid command**.
  - pentru toate comenzile, inainte de a se trimite ceva la server, se verifica daca datele introduse de utilizator sunt valide. De exemplu, page_count ar trebui sa contina numai cifre, iar stringurile sa nu fie goale.