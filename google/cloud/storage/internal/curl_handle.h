// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_INTERNAL_CURL_HANDLE_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_INTERNAL_CURL_HANDLE_H_

#include "google/cloud/storage/internal/curl_wrappers.h"
#include <curl/curl.h>

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
namespace internal {
/**
 * A wrapper around CURL* handles.
 *
 * This is a fairly straightforward wrapper around the CURL* handle. It provides
 * nicer C++-style API for the curl_*() functions, and some helpers to ease
 * the use of the API.
 */
class CurlHandle {
 public:
  explicit CurlHandle();
  ~CurlHandle();

  // This class holds unique ptrs, disable copying.
  CurlHandle(CurlHandle const&) = delete;
  CurlHandle& operator=(CurlHandle const&) = delete;

  // Allow moves, they immediately disable callbacks.
  CurlHandle(CurlHandle&& rhs) : handle_(std::move(rhs.handle_)) {
    ResetHeaderCallback();
    ResetReaderCallback();
    ResetWriterCallback();
  }
  CurlHandle& operator=(CurlHandle&& rhs) {
    handle_ = std::move(rhs.handle_);
    ResetHeaderCallback();
    ResetReaderCallback();
    ResetWriterCallback();
    return *this;
  }

  /**
   * Define the callback type for sending data.
   *
   * In the conventions of libcurl, the read callbacks are invoked by the
   * library to gather more data to send to the server.
   *
   * @see https://curl.haxx.se/libcurl/c/CURLOPT_READFUNCTION.html
   */
  using ReaderCallback = std::function<std::size_t(char* ptr, std::size_t size,
                                                   std::size_t nmemb)>;

  /**
   * Define the callback type for receiving data.
   *
   * In the conventions of libcurl, the write callbacks are invoked by the
   * library when more data has been received.
   *
   * @see https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
   */
  using WriterCallback = std::function<std::size_t(void* ptr, std::size_t size,
                                                   std::size_t nmemb)>;

  /**
   * Define the callback type for receiving header data.
   *
   * In the conventions of libcurl, the header callbacks are invoked when new
   * header-like data is received.
   *
   * @see https://curl.haxx.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
   */
  using HeaderCallback = std::function<std::size_t(
      char* contents, std::size_t size, std::size_t nitems)>;

  /**
   * Set the reader callback.
   *
   * @param callback this function must remain valid until either
   *     `ResetReaderCallback` returns, or this object is destroyed.
   *
   * @see the notes on `ReaderCallback` for the semantics of the callback.
   */
  void SetReaderCallback(ReaderCallback callback);

  /// Reset the reader callback.
  void ResetReaderCallback();

  /**
   * Set the writer callback.
   *
   * @param callback this function must remain valid until either
   *     `ResetWriterCallback` returns, or this object is destroyed.
   *
   * @see the notes on `WriterCallback` for the semantics of the callback.
   */
  void SetWriterCallback(WriterCallback callback);

  /// Reset the reader callback.
  void ResetWriterCallback();

  /**
   * Set the header callback.
   *
   * @param callback this function must remain valid until either
   *    `ResetHeaderCallback` returns, or this object is destroyed
   *
   * @see the notes on `ReaderCallback` for the semantics of the callback.
   */
  void SetHeaderCallback(HeaderCallback callback);

  /// Reset the reader callback.
  void ResetHeaderCallback();

  /// URL-escape a string.
  CurlString MakeEscapedString(std::string const& s) {
    return CurlString(
        curl_easy_escape(handle_.get(), s.data(), static_cast<int>(s.length())),
        &curl_free);
  }

  template <typename T>
  void SetOption(CURLoption option, T&& param) {
    auto e = curl_easy_setopt(handle_.get(), option, std::forward<T>(param));
    if (e == CURLE_OK) {
      return;
    }
    RaiseSetOptionError(e, option, std::forward<T>(param));
  }

  void EasyPerform() {
    auto e = curl_easy_perform(handle_.get());
    if (e == CURLE_OK) {
      return;
    }
    RaiseError(e, __func__);
  }

  long GetResponseCode() {
    long code;
    auto e = curl_easy_getinfo(handle_.get(), CURLINFO_RESPONSE_CODE, &code);
    if (e != CURLE_OK) {
      RaiseError(e, __func__);
    }
    return code;
  }

  void EasyPause(int bitmask) {
    auto e = curl_easy_pause(handle_.get(), bitmask);
    if (e != CURLE_OK) {
      RaiseError(e, __func__);
    }
  }

  void EnableLogging(bool enabled);

  /// Flush any debug data using GCP_LOG().
  void FlushDebug(char const* where);

 private:
  friend class CurlDownloadRequest;
  friend class CurlRequest;
  friend class CurlUploadRequest;
  friend class CurlRequestBuilder;

  [[noreturn]] void RaiseError(CURLcode e, char const* where);
  [[noreturn]] void RaiseSetOptionError(CURLcode e, CURLoption opt, long param);
  [[noreturn]] void RaiseSetOptionError(CURLcode e, CURLoption opt,
                                        char const* param);
  [[noreturn]] void RaiseSetOptionError(CURLcode e, CURLoption opt,
                                        void* param);
  template <typename T>
  [[noreturn]] void RaiseSetOptionError(CURLcode e, CURLoption opt, T unused) {
    std::string param = "complex-type=<";
    param += typeid(T).name();
    param += ">";
    RaiseSetOptionError(e, opt, param.c_str());
  }

  using CurlPtr = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;
  CurlPtr handle_;
  std::string debug_buffer_;

  ReaderCallback reader_callback_;
  WriterCallback writer_callback_;
  HeaderCallback header_callback_;
};

}  // namespace internal
}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_INTERNAL_CURL_HANDLE_H_
