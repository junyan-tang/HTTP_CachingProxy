FROM ubuntu:20.04
RUN apt-get update
RUN apt-get install -y g++ make
RUN apt-get install -y libboost-all-dev
RUN mkdir /var/log/erss
RUN touch /var/log/erss/proxy.log
RUN mkdir /code
ADD . /code
WORKDIR /code
RUN chmod +x run.sh
