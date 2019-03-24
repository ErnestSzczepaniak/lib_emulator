#include "emulator.h"

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