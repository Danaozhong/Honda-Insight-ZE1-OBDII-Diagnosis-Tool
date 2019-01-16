#ifndef _COM_CHANNEL_HPP_
#define _COM_CHANNEL_HPP_

namespace driver
{
	enum ComChannelType
	{
		COM_CHANNEL_RX,
		COM_CHANNEL_TX,
		COM_CHANNEL_RX, TX

	};

	class ComChannel
	{
	public:
		virtual ~ComChannel();

		virtual int open() = 0;
		virtual int close() = 0;

	protected:
		ComChannelType m_en_com_channel;
	};

	class ComChannelTx : public ComChannel
	{
	public:
		virtual void v_send(char* ai8_buffer, unsigned int u32_buffer_size) = 0;
	};

	class ComChannelRx : public ComChannel
	{
	public:
		virtual void v_receive(char* ai8_buffer, unsigned int u32_buffer_size) = 0;
	};

}

#endif _COM_CHANNEL_HPP_
