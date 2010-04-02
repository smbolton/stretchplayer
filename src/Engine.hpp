/*
 * Copyright(c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <jack/jack.h>
#include <memory>
#include <QString>
#include <QMutex>
#include <vector>
#include <set>

namespace RubberBand
{
    class RubberBandStretcher;
}

namespace StretchPlayer
{

class EngineMessageCallback;

class Engine
{
public:
    Engine();
    ~Engine();

    void load_song(const QString& filename);
    void play();
    void play_pause();
    void stop();
    bool playing() {
	return _playing;
    }
    float get_position(); // in seconds
    float get_length();   // in seconds
    void locate(double secs);
    float get_stretch() {
	return _stretch;
    }
    void set_stretch(float str) {
	if(str > 0.5 && str < 2.0) {
	    _stretch = str;
	}
    }
    int get_pitch() {
	return _pitch;
    }
    void set_pitch(int pit) {
	if(pit < -12) {
	    _pitch = -12;
	} else if (pit > 12) {
	    _pitch = 12;
	} else {
	    _pitch = pit;
	}
    }

    void subscribe_errors(EngineMessageCallback* obj) {
	_subscribe_list(_error_callbacks, obj);
    }
    void unsubscribe_errors(EngineMessageCallback* obj) {
	_unsubscribe_list(_error_callbacks, obj);
    }
    void subscribe_messages(EngineMessageCallback* obj) {
	_subscribe_list(_message_callbacks, obj);
    }
    void unsubscribe_messages(EngineMessageCallback* obj) {
	_unsubscribe_list(_message_callbacks, obj);
    }

private:
    static int static_jack_callback(jack_nframes_t nframes, void* arg) {
	Engine *e = static_cast<Engine*>(arg);
	return e->jack_callback(nframes);
    }

    int jack_callback(jack_nframes_t nframes);

    void _zero_buffers(jack_nframes_t nframes);
    void _process_playing(jack_nframes_t nframes);

    typedef std::set<EngineMessageCallback*> callback_seq_t;

    void _error(const QString& msg) const {
	_dispatch_message(_error_callbacks, msg);
    }
    void _message(const QString& msg) const {
	_dispatch_message(_message_callbacks, msg);
    }
    void _dispatch_message(const callback_seq_t& seq, const QString& msg) const;
    void _subscribe_list(callback_seq_t& seq, EngineMessageCallback* obj);
    void _unsubscribe_list(callback_seq_t& seq, EngineMessageCallback* obj);

    jack_client_t* _jack_client;
    jack_port_t *_port_left, *_port_right;

    bool _playing;
    mutable QMutex _audio_lock;
    std::vector<float> _left;
    std::vector<float> _right;
    unsigned long _position;
    float _sample_rate;
    float _stretch;
    int _pitch;
    std::auto_ptr<RubberBand::RubberBandStretcher> _stretcher;

    mutable QMutex _callback_lock;
    callback_seq_t _error_callbacks;
    callback_seq_t _message_callbacks;

}; // Engine

class EngineMessageCallback
{
public:
    virtual ~EngineMessageCallback() {
	if(_parent) {
	    _parent->unsubscribe_errors(this);
	}
	if(_parent) {
	    _parent->unsubscribe_messages(this);
	}
    }

    virtual void operator()(const QString& message) = 0;

private:
    friend class Engine;
    Engine *_parent;
};

} // namespace StretchPlayer

#endif // ENGINE_HPP