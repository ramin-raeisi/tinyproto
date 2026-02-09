#pragma once

#include "TinySerialLinkLayer.h"
#include "TinyFdLinkLayer.h"
#include <interface/SinkSourceInterface.hpp>

#if defined(ARDUINO)
#include "proto/fd/tiny_fd_int.h"
#endif

namespace tinyproto
{

template <int MaxBlockSize>
class CustomSerialFdLink: public ISerialLinkLayer<IFdLinkLayer, MaxBlockSize>
{
public:
    using AllocFunc = void*(*)(size_t);
    using FreeFunc  = void(*)(void*);

    explicit CustomSerialFdLink(ISource &rx, ISink &tx,
                                AllocFunc allocFn = defaultAlloc,
                                FreeFunc freeFn = defaultFree)
        : ISerialLinkLayer<IFdLinkLayer, MaxBlockSize>(const_cast<char *>(""), nullptr, 0)
        , _rx(rx)
        , _tx(tx)
        , m_alloc(allocFn)
        , m_free(freeFn)
    {
    }

    ~CustomSerialFdLink()
    {
        if ( m_buffer )
        {
            m_free(m_buffer);
            m_buffer = nullptr;
        }
    }

    bool begin(on_frame_read_cb_t onReadCb, on_frame_send_cb_t onSendCb, void *udata) override
    {
        int size = tiny_fd_buffer_size_by_mtu_ex(1, this->getMtu(), this->getWindow(), this->getCrc(), 3);
        m_buffer = reinterpret_cast<uint8_t *>(m_alloc(size));
        this->setBuffer(m_buffer, size);
        return ISerialLinkLayer<IFdLinkLayer, MaxBlockSize>::begin(onReadCb, onSendCb, udata);
    }

    void end() override
    {
        ISerialLinkLayer<IFdLinkLayer, MaxBlockSize>::end();
        if ( m_buffer )
        {
            m_free(m_buffer);
            m_buffer = nullptr;
        }
    }

    void runRx() override
    {
        uint8_t buf[MaxBlockSize];
        uint8_t *p = buf;

        int len = _rx.receive(p, MaxBlockSize);
        while ( len > 0 )
        {
            int temp = IFdLinkLayer::parseData(p, len);
            if ( temp < 0 )
            {
                break;
            }
            len -= temp;
            p += temp;
        }
    }

    void runTx() override
    {
        uint8_t buf[MaxBlockSize];
        int len = IFdLinkLayer::getData(buf, MaxBlockSize);
        uint8_t *ptr = buf;
        while ( len > 0 )
        {
            int sent = _tx.send(ptr, len);
            if ( sent < 0 )
            {
                break;
            }
            ptr += sent;
            len -= sent;
        }
    }

private:
    static void* defaultAlloc(size_t size) { return ::operator new(size); }
    static void  defaultFree(void* ptr)    { ::operator delete(ptr); }

    uint8_t *m_buffer = nullptr;
    ISource &_rx;
    ISink &_tx;
    AllocFunc m_alloc;
    FreeFunc  m_free;
};

} // namespace tinyproto
