FROM nubit/base
RUN apt-get install -y wget
RUN mkdir -p /code
RUN wget https://nubits.com/sites/default/files/assets/nud-0.4.2-test3 -O /code/nud
RUN sha1sum /code/nud | grep 5c3289eb10b0f1ae15adad91ad8ba183e95612fb
RUN chmod +x /code/nud
