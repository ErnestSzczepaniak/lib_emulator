#ifndef _emulator_h
#define _emulator_h
// stara wersja
#include <fcntl.h> 
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include "string.h"
#include <typeindex>

template<typename ... Args>
void _d(const char * format, Args ... args)
{
    char buffer[128];

    //snprintf(buffer, 127, format, args ...);

    std::cout << buffer << std:: endl;
}

// ######################################### EMULATOR ####################################################3

class Emulator
{
    struct Map{bool reqne; int reqid; char request[4096]; bool respne; int respid; char response[4096];};

protected:
    void _init(const char * name);

    void _request_send(int id, void * from, int size);
    int _request_receive();
    
    void _response_send();
    void _response_receive(void * to, int size);

    Map * _map;
};

void Emulator::_init(const char * name)
{
        _d("Starting local emulator with name # %s #", name);

        if (auto descriptor = shm_open(name, O_RDWR | O_CREAT, 0644); descriptor >= 0)
        {
            _d("File descriptor aquired !");
            _d("Truncating to map size of (%d) bytes ...", sizeof(Map));

            if (auto result = ftruncate(descriptor, sizeof(Map)); result != -1)
            {
                _d("Truncation successful !");
                _d("Mapping to process address space ...");

                if (_map = (Map*)mmap(NULL, sizeof(Map), PROT_READ|PROT_WRITE, MAP_SHARED, descriptor, 0); _map != nullptr)
                {
                    _d("Mapping successful !");
                    _d("Shared memory created");
                }
                else
                {
                    _d("Unable to map to process address space :(");
                }
            }
            else
            {
                _d("Failed to truncate to map size :(");
                _d("Mapping to process address space ...");

                if (_map = (Map*)mmap(NULL, sizeof(Map), PROT_READ|PROT_WRITE, MAP_SHARED, descriptor, 0); _map != nullptr)
                {
                    _d("Mapping successful !");
                    _d("Shared memory created");
                }
                else
                {
                    _d("Unable to map to process address space :(");
                }
            }
        }
        else
        {
            _d("Unable to aquire file descriptor :(");
        }
}

void Emulator::_request_send(int id, void * from, int size)
{
    while(_map->reqne);
    _map->reqid = id;
    memcpy(_map->request, from, size);
    _map->reqne = 1;
}

int Emulator::_request_receive()
{
    while(!_map->reqne);
    return _map->reqid;
}

void Emulator::_response_send()
{
    _map->respne = 1;
    _map->reqne = 0;
}

void Emulator::_response_receive(void * to, int size)
{
    while(!_map->respne);
    memcpy(to, _map->response, size);
    _map->respne = 0;
}

// ############################################## EMULATOR SERVER #############################################################

template<typename ... T>
class Emulator_server
:
public Emulator
{
    using Handler = void (*)(void *, void *);

public:
    Emulator_server(const char * name, T  ... t);
    ~Emulator_server();

    void start();

protected:

private:
    Handler * _handler;
};

template<typename ...T>
Emulator_server<T...>::Emulator_server(const char * name, T ... t)
{
    _d("Creating emulator server, aquaring provided handlers ...");

    std::type_index types[] = {typeid(T)...};

    _d("Server has %d valid handlers:", sizeof...(T));

    for (unsigned int i = 0; i < sizeof...(T); i++)
    {
        _d("Handler %d = %s", i, types[i].name());
    }

    _handler = new Handler[sizeof...(T)]{(Handler)t...};

    _init(name);
}

template<typename ...T>
Emulator_server<T...>::~Emulator_server()
{
    delete[] _handler;
}

template<typename ...T>
void Emulator_server<T...>::start()
{
    _d("Server started, waiting for user requests ...");

    while(1)
    {
        auto id = _request_receive();

        _handler[id](_map->request, _map->response);

        _response_send();
    }
}

// ############################################## EMULATOR CLIENT #############################################################

template<typename ... T>
class Emulator_client
:
public Emulator
{
    template<typename U>
    class Req_type
    {
    public:
        static int id;
    };

public:
    Emulator_client(const char * name);
    ~Emulator_client();

    template<typename V, typename ...Args> void send(Args ... args);
    template<typename V> void send(const V & payload);

    template<typename V> V receive();

protected:
    

private:
    

};

template<typename ...T>
template<typename U>
int Emulator_client<T...>::Req_type<U>::id = -1;


template<typename ...T>
Emulator_client<T...>::Emulator_client(const char * name)
{
    int index = 0;

    ((Req_type<T>::id = index++), ...);

    _init(name);
}

template<typename ...T>
Emulator_client<T...>::~Emulator_client()
{

}

template<typename ...T>
template<typename V> 
void Emulator_client<T...>::send(const V & payload)
{
    _request_send(Req_type<V>::id, (void *)&payload, sizeof(V));
}

template<typename ...T>
template<typename V, typename ...Args> 
void Emulator_client<T...>::send(Args ... args)
{
    V payload = {args...};

    _request_send(Req_type<V>::id, &payload, sizeof(V));
}

template<typename ...T>
template<typename V>
V Emulator_client<T...>::receive()
{
    V temp;

    _response_receive(&temp, sizeof(V));

    return temp;
}

#endif
