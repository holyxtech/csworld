#ifndef V8_HANDLER_H
#define V8_HANDLER_H

#include "include/cef_base.h"
#include "include/cef_v8.h"
#include "include/wrapper/cef_helpers.h"

class V8Handler : public CefV8Handler {
public:
  V8Handler(CefRefPtr<CefBrowser> browser);
  bool Execute(
    const CefString& name,
    CefRefPtr<CefV8Value> object,
    const CefV8ValueList& arguments,
    CefRefPtr<CefV8Value>& retval,
    CefString& exception) override;

private:
  CefRefPtr<CefListValue> ConvertV8ArrayToList(CefRefPtr<CefV8Value> v8Array);
  CefRefPtr<CefDictionaryValue> ConvertV8ObjectToDictionary(CefRefPtr<CefV8Value> v8Object);
  CefRefPtr<CefValue> ConvertV8ValueToCefValue(CefRefPtr<CefV8Value> value);

  CefRefPtr<CefBrowser> browser_;

  IMPLEMENT_REFCOUNTING(V8Handler);
};

#endif