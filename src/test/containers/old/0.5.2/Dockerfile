FROM nubit/base
RUN apt-get install -y wget
RUN mkdir -p /code
RUN wget https://nubits.com/sites/default/files/assets/nud-0.5.2-test -O /code/nud
RUN sha1sum /code/nud | grep 3810b1bd4200450625c149382eab4e3ab1dae7b4
RUN chmod +x /code/nud
