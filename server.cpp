#include <ev++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <thread>
#include <queue>
#include <vector>

using namespace std;
using namespace ev;

size_t total_clients=1;
size_t countt=0;

vector <pair<int,int>> vect;

void read_cb(struct ev_loop *loop,struct ev_io *watcher, int revents){
    char bf[4]={0};
    read(watcher->fd,&bf,1);
    size_t rzm=stoul(bf);
    bzero(bf,4);

    read(watcher->fd,&bf,rzm);

    bool flag=true;

    int nfd=stoi(bf);

    string ubuff;
    char buffer[50000]={0};
    ssize_t readd;

    if(EV_ERROR & revents){
        perror("everror");
        exit(EXIT_FAILURE);
    }

    while (flag==true){
        cout<<"!!!working with!!!: "<<bf<<"on: "<<watcher->fd<<endl;
        readd=recv(nfd,&buffer,50000,0);
        cout<<"READDDDDDDD: "<<readd<<endl;

        if(readd<0){
            perror("\nrecv did nothing\n");
            return;
        }

        if (readd==0){
            flag=false;

            if(shutdown(nfd,SHUT_RDWR)){
                perror("shutdown error");
            };
            return;
        }else {
            ubuff.append(buffer,readd);
            cout<<"rssize now:"<<ubuff.length()<<endl;
            if(buffer[readd-1]=='\r'){
                int abc=rand();
                string bufr=to_string(abc);
                size_t k1=0;
                size_t k2=0;
                k1= ubuff.find("REQUEST_URI");
                k2= ubuff.find("QUERY_STRING");
                string kk;
                kk.append("HTTP/1.1 200 OK\nContent-Type: text/html\nContent-length: 20\n\n<h1>");
                kk.append(ubuff.substr(k1+sizeof("REQUEST_URI"),k2-(k1+sizeof("REQUEST_URI"))));
                kk.append(bufr);
                if (ubuff.length()>560){ //more useless data
                    kk.append(ubuff.substr(560,ubuff.length()-600));
                }
                kk.append("</h1>\r");
                ssize_t sentt = send(nfd,kk.c_str(),kk.length(),0);
                if(sentt<=0){
                    perror("send error");
                    exit(EXIT_FAILURE);
                }
                cout<<"peer might closing for "<<nfd<<"sent: "<<sentt<<endl;
                ubuff.erase();
            }
        }
        bzero(buffer,50000);
    }
}

class inito{
    public:
    int server_fd;
    int opt = 1;

    void sockstart(int port){

        struct sockaddr_in address;

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr("127.0.0.1");
        address.sin_port = htons( port );

        if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
            perror("sockerror");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopterror");
            exit(EXIT_FAILURE);
        }
        if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address))<0) {
            perror("binderror");
            exit(EXIT_FAILURE);
        }
    }
};

class Work{
public:
size_t num;
    Work(size_t i){
        num=i;

        start();
    }

    ~Work(){
        ev_io_stop(loop2,&watch);
        ev_break (loop2, EVBREAK_ALL);
        ev_loop_destroy(loop2);
    }

private:

    void start(){
        cout<<"working at numb: "<< num<<endl;

        loop2=ev_loop_new(EVFLAG_AUTO);

        ev_io_init(&watch,read_cb,vect.at(num).first,EV_READ);
        ev_io_start(loop2,&watch);

        //watch.set<read_cb>();
        //watch.set(loop2);
        //watch.start(vect.at(num).first,READ);

        ev_run(loop2,0);
    }

    struct ev_loop *loop2; // because there is no way I can create a loop_new with C++ API
    ev_io watch;
};


void stdin_cb (struct ev_loop *loop,struct ev_io *watcher, int revents) {
    ev_io_stop (loop, watcher);
    ev_break (loop, EVBREAK_ALL);
}

void accept_cb(struct ev_loop *loop,struct ev_io *watcher, int revents){
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sd;

    if(EV_ERROR & revents) {
        perror("got invalid event");
        exit(EXIT_FAILURE);
    }

    client_sd = accept(watcher->fd,reinterpret_cast<struct sockaddr *>(&client_addr),&client_len);
    if (client_sd <0 ){
        perror("accept error");
        exit(EXIT_FAILURE);
    }

    cout<< total_clients << " clients connected!"<<endl;

    size_t ct=total_clients%(countt);
    total_clients++;

    cout<<"not in main with "<<client_sd<<"to "<<vect.at(ct).second<<" RD FRM: "<<vect.at(ct).first<<endl;

    string another_unnecessary_string;
    another_unnecessary_string.append(to_string(to_string(client_sd).length()));
    another_unnecessary_string.append(to_string(client_sd));
    write(vect.at(ct).second,another_unnecessary_string.c_str(),another_unnecessary_string.length());

}

int main(int argc, char **argv){
    if (argc <3){
        perror("argc error: ");
        return 1;
    }

    string argo=argv[1];
    countt = stoul(argo);

    inito starto; //обёртка на сокет

    string buf=argv[2];
    int port=stoi(buf);

    starto.sockstart(port); //set the inputed port and start socket


    vector<thread> threads; //vector of threads
    threads.reserve(countt-1); // reserve inputed amount of threads
    vect.reserve(countt);

    for(size_t i=0;i<countt;++i){ // make full vector of new watchers and loops for workers
        int fdp[2];
        pipe(fdp);
        vect.emplace_back(make_pair(fdp[0],fdp[1]));
    }

    vector<unique_ptr<Work>> n;
    for(size_t i2=0;i2<countt-1;++i2){ // make full vector of new worker objects
        cout<<" i2: "<<i2<<endl;
        threads.emplace_back([&n,i2]{
            n.push_back(make_unique<Work>(i2));
        });
    }

    if (listen(starto.server_fd, 1000) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    default_loop loop; // our main loop
    //В плюсовом варианте loop вроде вшит уже в вотчер. Далее цитата из документации:
    //w->start ()
    //Starts the watcher. Note that there is no loop argument, as the constructor already stores the event loop.
    // Но нам нужно прикрепить несколько вотчеров к одной. Поэтому лучше использовать дефолтную.

    ev_io w_accept; // our accept watcher
    ev_io stdin_watcher; //our input watcher (if signal from input comes - call smth)
    ev_io w_work;


    ev_io_init(&w_accept,accept_cb,starto.server_fd,EV_READ);
    ev_io_start(loop,&w_accept);

    ev_io_init(&stdin_watcher,stdin_cb,0,EV_READ);
    ev_io_start(loop,&stdin_watcher);

    ev_io_init(&w_work,read_cb,vect.at(countt-1).first,EV_READ);
    ev_io_start(loop,&w_work);

    ev_run(loop,0); // start loop

    ev_loop_destroy(loop); // destroy it

    return(0);
}
