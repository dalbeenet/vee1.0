#ifndef _VEE_OBJECT_POOL_H_
#define _VEE_OBJECT_POOL_H_

#include <vee/queue.h>
#include <atomic>
#include <mutex>
#include <array>
#include <vector>

_VEE_BEGIN

template <class T, std::size_t ChunkSize = 1000>
class object_pool
{
public:
    static const std::size_t chunksize = ChunkSize;
    typedef T data_type;
    struct chunk
    {
        std::array<data_type, chunksize> container;
        vee::syncronized_ringqueue<std::size_t> queue;
        chunk()
        {
            init();
        }
        ~chunk() = default;
        chunk(const chunk& other)
        {

        }
        chunk& operator=(const chunk& other)
        {

        }
        void init()
        {
            for (int i = 0; i < chunksize; ++i)
                queue.enqueue(i);
        }
    };
    object_pool(unsigned int init_chunks)
    {
        for (int i = 0; i < init_chunks; ++i)
            _add_chunk();
    }
    ~object_pool()
    {
        
    }
protected:
    std::size_t _add_chunk()
    {
        std::lock_guard<std::mutex> guard(_mtx);
        _chunks.push_back(chunk_type());
        std::size_t ret = _size++;
        return ret;
    }
protected:
    std::atomic<std::size_t> _size = 0;
    std::vector<chunk> _chunks;
    std::mutex _mtx;
};

_VEE_END

#endif // _VEE_OBJECT_POOL_H_