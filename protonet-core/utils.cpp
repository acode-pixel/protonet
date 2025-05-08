#include <uv.h>
#include <sodium.h>
#include "./proto/utils.hpp"

int getFileHashSHA256(char* filepath, uv_loop_t* loop, void* out){
    uv_fs_t req;
    uv_fs_open(loop, &req, filepath, O_RDONLY, 0, NULL);

    if(req.result < 0){
        log_error("failed to read requested file %s.[%s]", filepath, uv_err_name(req.result));
		log_debug("failed to read requested file %s.[%s]", filepath, uv_strerror(req.result));
        uv_fs_req_cleanup(&req);
        return -1;
    }

	int file = req.result;
	char* data = (char*)malloc(65536);
	uv_buf_t buf = uv_buf_init(data, 65536);
	uv_fs_req_cleanup(&req);

	//CryptoPP::SHA256 hash;
	unsigned char hash[crypto_generichash_BYTES];
	crypto_generichash_state state;
	crypto_generichash_init(&state, NULL, 0, sizeof hash);
	uv_fs_read(loop, &req, file, &buf, 1, -1, NULL);

	while(req.result > 0){
		//hash.Update((const CryptoPP::byte*)buf.base, req.result);
		crypto_generichash_update(&state, (const unsigned char*)data, req.result);
		uv_fs_req_cleanup(&req);
		uv_fs_read(loop, &req, file, &buf, 1, -1, NULL);
	}

	uv_fs_req_cleanup(&req);
	uv_fs_close(loop, &req, file, NULL);
	uv_fs_req_cleanup(&req);

	//hash.Final((CryptoPP::byte*)out);
	crypto_generichash_final(&state, hash, sizeof hash);
	free(data);
	strncpy((char*)out, (const char*)hash, crypto_generichash_BYTES);
    return 0;

	/*CryptoPP::HexEncoder encoder;
	string encoded;
	encoder.Put(client->trac.hash, sizeof(client->trac.hash));
	encoder.MessageEnd();
	encoded.resize(sizeof(client->trac.hash));
	encoder.Get((CryptoPP::byte*)&encoded[0], encoded.size());*/
    
}

void getHex(uint8_t* hex, int size, char* out){
	std::stringstream ss;
	ss << std::hex;

	for (size_t i = 0; i < size; i++) {
        ss << std::setw(2) << std::setfill('0') << (int)hex[i];
    }

	strcpy(out, ss.str().c_str());
}