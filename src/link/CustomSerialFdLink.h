#pragma once

#include "TinySerialLinkLayer.h"
#include "TinyFdLinkLayer.h"
#include <interface/SinkSourceInterface.hpp>

#if defined(ARDUINO)
#include "proto/fd/tiny_fd_int.h"
#endif

namespace tinyproto
{

static constexpr int maximumBlockSize = 32;

class CustomSerialFdLink: public ISerialLinkLayer<IFdLinkLayer, maximumBlockSize>
{
public:
    explicit CustomSerialFdLink(ISource &rx, ISink &tx)
        : ISerialLinkLayer<IFdLinkLayer, maximumBlockSize>(const_cast<char *>(""), nullptr, 0)
        , _rx(rx)
        , _tx(tx)
    {
    }

    ~CustomSerialFdLink();

    bool begin(on_frame_read_cb_t onReadCb, on_frame_send_cb_t onSendCb, void *udata) override;

    void end() override;

    void runRx() override;

    void runTx() override;

private:
    uint8_t *m_buffer = nullptr;
    ISource &_rx;
    ISink &_tx;
};

} // namespace tinyproto
