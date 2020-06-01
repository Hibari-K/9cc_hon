FROM ubuntu:18.04

#RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd

RUN apt -y update \
 && apt -y upgrade \
 && apt install -y git less vim sudo iproute2 net-tools apt-utils zsh \
                   openssh-server python3-pip gcc make binutils libc6-dev

# Create user
ARG USERNAME=hibari

RUN useradd -m $USERNAME \
 && echo $USERNAME:$USERNAME | chpasswd \
 && usermod -aG sudo $USERNAME \
 && echo $USERNAME | chsh -s /bin/zsh

RUN apt -y install gdb

USER $USERNAME
RUN git clone https://github.com/longld/peda.git /home/$USERNAME/peda
RUN echo "source ~/peda/peda.py" >> /home/$USERNAME/.gdbinit
