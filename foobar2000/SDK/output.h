#pragma once

#include <functional>

PFC_DECLARE_EXCEPTION(exception_output_device_not_found, pfc::exception, "Audio device not found")
PFC_DECLARE_EXCEPTION(exception_output_module_not_found, exception_output_device_not_found, "Output module not found")
PFC_DECLARE_EXCEPTION(exception_output_invalidated, pfc::exception, "Audio device invalidated")
PFC_DECLARE_EXCEPTION(exception_output_device_in_use, pfc::exception, "Audio device in use")
PFC_DECLARE_EXCEPTION(exception_output_unsupported_stream_format, pfc::exception, "Unsupported audio stream format")

//! Output interrupted due to another application taking exclusive access to the device - do not complain to user.
PFC_DECLARE_EXCEPTION(exception_output_interrupted, pfc::exception, "Output interrupted");

//! Structure describing PCM audio data format, with basic helper functions.
struct t_pcmspec
{
	unsigned m_sample_rate = 0;
	unsigned m_bits_per_sample = 0;
	unsigned m_channels = 0,m_channel_config = 0;
	bool m_float = false;

	inline unsigned align() const {return (m_bits_per_sample / 8) * m_channels;}

	uint64_t time_to_bytes(double p_time) const {return audio_math::time_to_samples(p_time,m_sample_rate) * (m_bits_per_sample / 8) * m_channels;}
	double bytes_to_time(uint64_t p_bytes) const {return (double) (p_bytes / ((m_bits_per_sample / 8) * m_channels)) / (double) m_sample_rate;}

	inline bool operator==(/*const t_pcmspec & p_spec1,*/const t_pcmspec & p_spec2) const
	{
		return /*p_spec1.*/m_sample_rate == p_spec2.m_sample_rate 
			&& /*p_spec1.*/m_bits_per_sample == p_spec2.m_bits_per_sample
			&& /*p_spec1.*/m_channels == p_spec2.m_channels
			&& /*p_spec1.*/m_channel_config == p_spec2.m_channel_config
			&& /*p_spec1.*/m_float == p_spec2.m_float;
	}

	inline bool operator!=(/*const t_pcmspec & p_spec1,*/const t_pcmspec & p_spec2) const
	{
		return !(*this == p_spec2);
	}

	inline void reset() { *this = t_pcmspec(); }
	inline bool is_valid() const
	{
		return m_sample_rate >= 1000 && m_sample_rate <= 1000000 &&
			m_channels > 0 && m_channels <= 256 && m_channel_config != 0 &&
			(m_bits_per_sample == 8 || m_bits_per_sample == 16 || m_bits_per_sample == 24 || m_bits_per_sample == 32);
	}
};

class NOVTABLE output_device_enum_callback
{
public:
	virtual void on_device(const GUID & p_guid,const char * p_name,unsigned p_name_length) = 0;
};

class NOVTABLE output : public service_base {
	FB2K_MAKE_SERVICE_INTERFACE(output,service_base);
public:
	//! Retrieves amount of audio data queued for playback, in seconds.
	virtual double get_latency() = 0;
	//! Sends new samples to the device. Allowed to be called only when update() indicates that the device is ready. \n
	//! update() should be called AGAIN after each process_samples() to know if the device is ready for more. \n
    //! This method SHOULD NOT block, only copy passed chunk and return immediately.
	virtual void process_samples(const audio_chunk & p_chunk) = 0;
	//! Updates playback; queries whether the device is ready to receive new data. \n
    //! This method SHOULD NOT block, only update internal state and return immediately.
	//! @param p_ready On success, receives value indicating whether the device is ready for next process_samples() call.
	virtual void update(bool & p_ready) = 0;
	//! Pauses/unpauses playback.
	virtual void pause(bool p_state) = 0;
	//! Flushes queued audio data. Called after seeking.
	virtual void flush() = 0;
	//! Forces playback of queued data. Called when there's no more data to send, to prevent infinite waiting if output implementation starts actually playing after amount of data in internal buffer reaches some level.
	virtual void force_play() = 0;
	
	//! Sets playback volume.
	//! @p_val Volume level in dB. Value of 0 indicates full ("100%") volume, negative values indciate different attenuation levels.
	virtual void volume_set(double p_val) = 0;
    
    //! Helper, see output_v4::is_progressing().
    bool is_progressing_();
    //! Helper, see output_v4::update_v2()
    size_t update_v2_();
    //! Helper, see output_v4::get_trigger_event()
    pfc::eventHandle_t get_trigger_event_();
	//! Helper, see output_v6::process_samples_v2()
	size_t process_samples_v2_(const audio_chunk&);

    //! Helper for output_entry implementation.
    static uint32_t g_extra_flags() { return 0; }

};

class NOVTABLE output_v2 : public output {
	FB2K_MAKE_SERVICE_INTERFACE(output_v2, output);
public:
	//! Obsolete, do not use.
	virtual bool want_track_marks() {return false;}
	//! Obsolete, do not use.
	virtual void on_track_mark() {}
	//! Obsolete, do not use.
	virtual void enable_fading(bool) { }
	//! Called when flushing due to manual track change rather than seek-within-track
	virtual void flush_changing_track() {flush();}
};

class dsp_chain_config;

//! \since 1.4
class NOVTABLE output_v3 : public output_v2 {
	FB2K_MAKE_SERVICE_INTERFACE(output_v3, output_v2);
public:
	//! Does this output require a specific sample rate? If yes, return the value, otherwise return zero. \n
	//! Returning a nonzero will cause a resampler DSP to be injected.
	virtual unsigned get_forced_sample_rate() { return 0; } 
	//! Allows the output to inject specific DSPs at the end of the used chain. \n
	//! Default implementation queries get_forced_sample_rate() and injects a resampler.
	virtual void get_injected_dsps( dsp_chain_config & );
};

//! \since 1.6
class NOVTABLE output_v4 : public output_v3 {
	FB2K_MAKE_SERVICE_INTERFACE(output_v4, output_v3);
public:
	//! Returns an event handle that becomes signaled once the output wants an update() call and possibly process_samples(). \n
	//! Optional; may return pfc::eventInvalid if not available at this time or not supported. \n
	//! Use the event only if update() signals that it cannot take any more data at this time. \n
	virtual pfc::eventHandle_t get_trigger_event() {return pfc::eventInvalid;}
	//! Returns whether the audio stream is currently being played or not. \n
	//! Typically, for a short period of time, initially sent data is not played until a sufficient amount is queued to initiate playback without glitches. \n
    //! For old outputs that do not implement this, the value can be assumed to be true.
	virtual bool is_progressing() = 0;
    
    //! Improved version of update(); returns 0 if the output isn't ready to receive any new data, otherwise an advisory number of samples - at the current stream format - that the output expects to take now. \n
    //! If the caller changes the stream format, the value is irrelevant. \n
    //! The output may return SIZE_MAX to indicate that it can take data but does not currently have any hints to tell how much. \n
    //! This method SHOULD NOT block, only update output state and return immediately.
    virtual size_t update_v2();
};

//! \since 1.6
class output_v5 : public output_v4 {
	FB2K_MAKE_SERVICE_INTERFACE(output_v5, output_v4);
public:
	virtual unsigned get_forced_channel_mask() { return 0; }
};

//! \since 2.2
class output_v6 : public output_v5 {
	FB2K_MAKE_SERVICE_INTERFACE(output_v6, output_v5);
public:
	//! Extended process_samples(), allowed to read only part of the chunk if out of buffer space to take whole.
	//! @returns Number of samples actually taken.
	virtual size_t process_samples_v2(const audio_chunk&) = 0;
};

//! \since 2.25
//! foobar2000 v2.25 functionality draft, use only for testing live info delivery in v2.25 beta.
class output_v7 : public output_v6 {
	FB2K_MAKE_SERVICE_INTERFACE(output_v7, output_v6);
public:
	//! Optional, gets notified about current playback metadata.
	//! @param audioSource can be any type (metadb_handle, metadb_info_container, possibly other), use operator &= to determine type.
	virtual void hint_source(fb2k::objRef audioSource) { (void)audioSource; }
};

class NOVTABLE output_entry : public service_base {
	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(output_entry);
public:
	//! Instantiates output class.
	virtual void instantiate(service_ptr_t<output> & p_out,const GUID & p_device,double p_buffer_length,bool p_dither,t_uint32 p_bitdepth) = 0;
	//! Enumerates devices supported by this output_entry implementation.
	virtual void enum_devices(output_device_enum_callback & p_callback) = 0;
	//! For internal use by backend. Each output_entry implementation must have its own guid.
	virtual GUID get_guid() = 0;
	//! For internal use by backend. Retrieves human-readable name of this output_entry implementation.
	virtual const char * get_name() = 0;

#ifdef _WIN32
	//! Obsolete, do not use.
	virtual void advanced_settings_popup(HWND,POINT) {}
#endif

	enum {
		flag_needs_bitdepth_config = 1 << 0,
		flag_needs_dither_config = 1 << 1,
		//! Obsolete, do not use.
		flag_needs_advanced_config = 1 << 2,
		flag_needs_device_list_prefixes = 1 << 3,

		//! Supports playing multiple simultaneous audio streams thru one device?
		flag_supports_multiple_streams = 1 << 4,

		//! High latency operation (such as remote network playback), mutually exclusive with flag_low_latency
		flag_high_latency = 1 << 5,
		//! Low latency operation (local playback), mutually exclusive with flag_high_latency
		flag_low_latency = 1 << 6,
        //! When set, the output will be used in special compatibility mode: guaranteed regular update() calls, injected padding (silence) at the end of stream.
        flag_needs_shims = 1 << 7,
	};

	virtual t_uint32 get_config_flags() = 0;

	uint32_t get_config_flags_compat();

	bool is_high_latency();
	bool is_low_latency();

	pfc::string8 get_device_name( const GUID & deviceID);
	bool get_device_name( const GUID & deviceID, pfc::string_base & out );

	static bool g_find( const GUID & outputID, output_entry::ptr & outObj );
	static output_entry::ptr g_find(const GUID & outputID );
};

//! Helper; implements output_entry for specific output class implementation. output_entry methods are forwarded to static methods of your output class. Use output_factory_t<myoutputclass> instead of using this class directly.
template<typename T, typename E = output_entry>
class output_entry_impl_t : public E
{
public:
	void instantiate(service_ptr_t<output> & p_out,const GUID & p_device,double p_buffer_length,bool p_dither,t_uint32 p_bitdepth) {
		p_out = new service_impl_t<T>(p_device,p_buffer_length,p_dither,p_bitdepth);
	}
	void enum_devices(output_device_enum_callback & p_callback) {T::g_enum_devices(p_callback);}
	GUID get_guid() {return T::g_get_guid();}
	const char * get_name() {return T::g_get_name();}

    t_uint32 get_config_flags() {
		t_uint32 flags = 0;
		if (T::g_advanced_settings_query()) flags |= output_entry::flag_needs_advanced_config;
		if (T::g_needs_bitdepth_config()) flags |= output_entry::flag_needs_bitdepth_config;
		if (T::g_needs_dither_config()) flags |= output_entry::flag_needs_dither_config;
		if (T::g_needs_device_list_prefixes()) flags |= output_entry::flag_needs_device_list_prefixes;
		if (T::g_supports_multiple_streams()) flags |= output_entry::flag_supports_multiple_streams;
		if (T::g_is_high_latency()) flags |= output_entry::flag_high_latency;
		else flags |= output_entry::flag_low_latency;
        flags |= T::g_extra_flags();
		return flags;
	}
};


//! Use this to register your output implementation.
template<class T>
class output_factory_t : public service_factory_single_t<output_entry_impl_t<T> > {};

//! Helper base class for output implementations. \n
//! This is the preferred way of implementing output. \n
//! This is NOT a public interface and its layout changes between foobar2000 SDK versions, do not assume other outputs to implement it.
class output_impl : public output_v7 {
protected:
	output_impl() {}
    
    //! Called periodically. You can update your state in this method. Can do nothing if not needed.
	virtual void on_update() = 0;
    //! Writes an audio chunk to your output. \n
    //! Will never get more than last can_write_samples() asked for. \n
    //! Format being send will match last open().
	virtual void write(const audio_chunk & p_data) = 0;
    //! @returns How many samples write() can take at this moment. \n
	//! It's called immediately after on_update() and can reuse value calculated in last on_update().
	virtual t_size can_write_samples() = 0;
    //! @returns Current latency, delay between last written sample and currently heard audio.
	virtual t_size get_latency_samples() = 0;
    //! Flush output, after seek etc.
	virtual void on_flush() = 0;
    //! Flush output due to manual track change in progress. \n
    //! Same as on_flush() by default.
	virtual void on_flush_changing_track() {on_flush();}
    //! Called before first chunk and on stream format change. \n
    //! Following write() calls will deliver chunks in the same format as specified here.
	virtual void open(audio_chunk::spec_t const & p_spec) = 0;
	

	//! Override this, not force_play(). \n
	//! output_impl will defer call to on_force_play() until out of data in its buffer.
	virtual void on_force_play() = 0;
    
    // base class virtual methods which derived class must also implement
    // virtual void pause(bool p_state) = 0;
    // virtual void volume_set(double p_val) = 0;
	// virtual bool is_progressing() = 0;
protected:
	void on_need_reopen() {m_active_spec.clear(); }
private:
	void flush() override final;
	void flush_changing_track() override final;
	void update(bool & p_ready) override final;
    size_t update_v2() override final;
	double get_latency() override final;
	void process_samples(const audio_chunk & p_chunk) override final;
	size_t process_samples_v2(const audio_chunk&) override final;
	void force_play() override final;
	void on_flush_internal();
	void send_force_play();

	bool queue_empty() const { return m_incoming_ptr == m_incoming.get_size(); }

	pfc::array_t<audio_sample,pfc::alloc_fast_aggressive> m_incoming;
	size_t m_incoming_ptr = 0, m_can_write = 0;
	audio_chunk::spec_t m_incoming_spec,m_active_spec;
	bool m_eos = false; // EOS issued by caller / no more data expected until a flush
	bool m_sent_force_play = false; // set if sent on_force_play()
};


class NOVTABLE volume_callback {
public:
    virtual void on_volume_scale(float v) = 0;
    virtual void on_volume_arbitrary(int v) = 0;
};

class NOVTABLE volume_control : public service_base {
	FB2K_MAKE_SERVICE_INTERFACE(volume_control, service_base)
public:
	virtual void add_callback(volume_callback * ptr) = 0;
	virtual void remove_callback(volume_callback * ptr) = 0;
	
	enum style_t {
		styleScale,
		styleArbitrary
	};

	virtual style_t getStyle() = 0;

	virtual float scaleGet() = 0;
	virtual void scaleSet(float v) = 0;

	virtual void arbitrarySet(int val) = 0;
	virtual int arbitraryGet() = 0;
	virtual int arbitraryGetMin() = 0;
	virtual int arbitraryGetMax() = 0;
	virtual bool arbitraryGetMute() = 0;
	virtual void arbitrarySetMute(bool val) = 0;
};


class NOVTABLE output_entry_v2 : public output_entry {
	FB2K_MAKE_SERVICE_INTERFACE(output_entry_v2, output_entry)
public:
	virtual bool get_volume_control(const GUID & id, volume_control::ptr & out) = 0;
	virtual bool hasVisualisation() = 0;
};

//! \since 1.5
class NOVTABLE output_devices_notify {
public:
    virtual void output_devices_changed() = 0;
protected:
    output_devices_notify() {}
private:
	output_devices_notify(const output_devices_notify &) = delete;
	void operator=(const output_devices_notify &) = delete;
};

//! \since 1.5
class NOVTABLE output_entry_v3 : public output_entry_v2 {
	FB2K_MAKE_SERVICE_INTERFACE(output_entry_v3, output_entry_v2)
public:

	//! Main thread only!
	virtual void add_notify(output_devices_notify *) = 0;
	//! Main thread only!
	virtual void remove_notify(output_devices_notify *) = 0;

	//! Main thread only!
	virtual void set_pinned_device(const GUID & guid) = 0;
};

#pragma pack(push, 1)
//! \since 1.3.5
struct outputCoreConfig_t {

	static outputCoreConfig_t defaults();

	GUID m_output;
	GUID m_device;
	double m_buffer_length;
	uint32_t m_flags;
	uint32_t m_bitDepth;
	enum { flagUseDither = 1 << 0, flagUseFades = 1 << 1 };
};
#pragma pack(pop)

//! \since 1.3.5
//! Allows components to access foobar2000 core's output settings.
class NOVTABLE output_manager : public service_base {
	FB2K_MAKE_SERVICE_COREAPI(output_manager);
public:
	//! Instantiates an output instance with core settings.
	//! @param overrideBufferLength Specify non zero to override user-configured buffer length in core settings.
	//! @returns The new output instance. Throws exceptions on failure (invalid settings or other).
	virtual output::ptr instantiateCoreDefault(double overrideBufferLength = 0) = 0;
	virtual void getCoreConfig( void * out, size_t outSize ) = 0;

	void getCoreConfig(outputCoreConfig_t& out);
	outputCoreConfig_t getCoreConfig();
};

//! \since 1.3.16
class NOVTABLE output_device_list_callback {
public:
	virtual void onDevice( const char * fullName, const GUID & output, const GUID & device ) = 0;
};

//! \since 1.3.16
class NOVTABLE output_config_change_callback {
public:
	virtual void outputConfigChanged() = 0;
};

//! \since 1.4
class NOVTABLE output_manager_v2 : public output_manager {
	FB2K_MAKE_SERVICE_COREAPI_EXTENSION(output_manager_v2, output_manager);
public:
	virtual void setCoreConfig( const void * in, size_t inSize, bool bSuppressPlaybackRestart = false ) = 0;
	void setCoreConfig( const outputCoreConfig_t & in ) { setCoreConfig(&in, sizeof(in) ); }
	virtual void setCoreConfigDevice( const GUID & output, const GUID & device ) = 0;
	virtual void listDevices( output_device_list_callback & callback ) = 0;
	void listDevices( std::function< void ( const char*, const GUID&, const GUID&) > f );
	virtual void addCallback( output_config_change_callback * ) = 0;
	virtual void removeCallback( output_config_change_callback * ) = 0;

	service_ptr addCallback( std::function<void()> f );
	void addCallbackPermanent( std::function<void()> f );
};

extern const GUID output_id_null;
extern const GUID output_id_default;
