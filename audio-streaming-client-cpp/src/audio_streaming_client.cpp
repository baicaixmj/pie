#include "audio_streaming_client.h"
#include <fstream>
#include <thread>
#include <sstream>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/impl/codegen/sync_stream.h>
#include "base64.h"

DEFINE_string(default_app_name, "my_sdk", "SKD's name");
DEFINE_bool(default_enable_long_speech, true, "enable long speech");
DEFINE_bool(default_enable_chunk, true, "enable chunk");
DEFINE_uint32(default_log_level, 5, "log level");
DEFINE_double(default_send_per_second, 0.16, "send per second");
DEFINE_double(default_sleep_ratio, 1, "sleep ratio");
DEFINE_int32(default_timeout, 100, "timeout");


namespace com {
namespace baidu {
namespace acu {
namespace pie {

AsrClient::AsrClient()
	: _set_enable_flush_data(false)
	, _set_product_id(false)
	, _inited(false)  {
    _init_request.set_app_name(FLAGS_default_app_name);
    _init_request.set_enable_long_speech(FLAGS_default_enable_long_speech);
    _init_request.set_enable_chunk(FLAGS_default_enable_chunk);
    _init_request.set_log_level(FLAGS_default_log_level);
    _init_request.set_send_per_seconds(FLAGS_default_send_per_second);
    _init_request.set_sleep_ratio(FLAGS_default_sleep_ratio);
}

void AsrClient::set_app_name(const std::string& app_name) {
    _init_request.set_app_name(app_name);
}

void AsrClient::set_enable_flush_data(bool enable_flush_data) {
    _init_request.set_enable_flush_data(enable_flush_data);
    _set_enable_flush_data = true;
}

void AsrClient::set_enable_long_speech(bool enable_long_speech){
    _init_request.set_enable_long_speech(enable_long_speech);
}

void AsrClient::set_enable_chunk(bool enable_chunk) {
    _init_request.set_enable_chunk(enable_chunk);
}

void AsrClient::set_log_level(int log_level) {
    _init_request.set_log_level(log_level);
}

void AsrClient::set_send_per_seconds(double send_per_seconds) {
    _init_request.set_send_per_seconds(send_per_seconds);
}

void AsrClient::set_sleep_ratio(double sleep_ratio) {
    _init_request.set_sleep_ratio(sleep_ratio);
}

void AsrClient::set_product_id(const std::string& product_id) {
    _init_request.set_product_id(product_id);
    _set_product_id = true;
}

int AsrClient::init(const std::string& address) {
    if (!_set_product_id || !_set_enable_flush_data) {
	std::cerr << "Missing required field `product_id` or `enable_flush_data`" << std::endl;
	return -1;
    }

    _channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    _inited = true;
    
    return 0;
}

AsrStream* AsrClient::get_stream() {
    AsrStream* asr_stream = new AsrStream();
    asr_stream->_stub = AsrService::NewStub(_channel);
    if (!asr_stream->_stub) {
        std::cerr << "[error] Fail to create stub in get_stream" << std::endl;
        return NULL;
    } else {
        std::cout << "[debug] Create stub success in get_stream" << std::endl;
    }
    asr_stream->_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(FLAGS_default_timeout));
    asr_stream->_context.AddMetadata("audio_meta", base64_encode(_init_request.SerializeAsString()));
    asr_stream->_stream = asr_stream->_stub->send(&(asr_stream->_context));
    if (!asr_stream->_stream) {
        std::cerr << "[error] Fail to create stream in get_stream" << std::endl;
        return NULL;
    } else {
        std::cout << "[debug] Create stream success in get_stream" << std::endl;	    
    }

    return asr_stream;
}

int AsrClient::destroy_stream(AsrStream* stream) {
    
    if (!stream) {
        std::cerr << "[error] stream not exits when destroy AsrStream" << std::endl;
    }
    if (stream->finish() != 0) {
        std::cerr << "[error] Fail to finish stream when destroy AsrStream" << std::endl;
    }
    delete stream;
    return 0;
}

AsrStream::AsrStream()
          : _stream(nullptr)
	  , _stub(nullptr)
          , _writesdone(false) {}

int AsrStream::write(const void* buffer, size_t size, bool is_last) {
    if (_writesdone) {
        std::cerr << "[error] write stream has been done" << std::endl;
	return -1;
    }
    int status = 0;
    if (size < 0) {
        std::cerr << "[error] size < 0 in  write stream" << std::endl;
        status = -1;
    } else {
    	com::baidu::acu::pie::AudioFragmentRequest request;
    	request.set_audio_data(buffer, size);
    	//std::cout << "[debug] will run _stream->Write(request) ... ..." << std::endl;
    	if (!_stream->Write(request)) {
	    std::cerr << "[error] Write to stream error" << std::endl;
    	    status = -1;
	}
    }
    if (is_last) {
        std::cout << "[debug] will run _stream->WritesDone" << std::endl;
	if (_stream->WritesDone()) {
	    std::cout << "[info] Write done" << std::endl;
	    _writesdone = true;
	} else {
	    std::cerr << "[error] Write done in stream error" << std::endl;
	    status = -1;
	}
    }
    return status;
}

int AsrStream::read(AsrStreamCallBack callback_fun, void* data) {
    AudioFragmentResponse response;
    //std::cout << "[debug] will run _stream->Read(&response) ... ..." << std::endl;
    if (_stream->Read(&response)) {
        //std::cout << "[debug] run callback_fun ... ..." << std::endl;
	callback_fun(response, data);
        return 0;
    } else {
        //std::cout << "[debug] _stream->Read return false" << std::endl;
        return -1;
    }
}

int AsrStream::finish() {
    std::cout << "will run _stream->Finish()" << std::endl;
    grpc::Status status = _stream->Finish();
    std::cout << "complete run _stream->Finish()" << std::endl;
    if (!status.ok()) {
        std::cerr << "[error] status stream->Finish returned is not ok" << std::endl;
	return -1; 
    }
    //std::cout << "[debug] Stream finished." << std::endl;
    return 0;
}

} // namespace pie
} // namespace acu
} // namespace baidu
} // namespace com
