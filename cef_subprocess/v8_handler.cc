#include "v8_handler.h"

V8Handler::V8Handler(CefRefPtr<CefBrowser> browser)
    : browser_(browser) {
}

bool V8Handler::Execute(
  const CefString& name,
  CefRefPtr<CefV8Value> object,
  const CefV8ValueList& arguments,
  CefRefPtr<CefV8Value>& retval,
  CefString& exception) {
  if (name == "sendMessage") {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("sendMessage");
    CefRefPtr<CefListValue> args = msg->GetArgumentList();

    for (size_t i = 0; i < arguments.size(); ++i) {
      args->SetValue(i, ConvertV8ValueToCefValue(arguments[i]));
    }

    // Send the process message to the browser process
    browser_->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);

    return true;
  }

  return false;
}

CefRefPtr<CefValue> V8Handler::ConvertV8ValueToCefValue(CefRefPtr<CefV8Value> value) {
  CefRefPtr<CefValue> cef_value = CefValue::Create();
  if (value->IsNull() || value->IsUndefined()) {
    cef_value->SetNull();
  } else if (value->IsBool()) {
    cef_value->SetBool(value->GetBoolValue());
  } else if (value->IsInt()) {
    cef_value->SetInt(value->GetIntValue());
  } else if (value->IsDouble()) {
    cef_value->SetDouble(value->GetDoubleValue());
  } else if (value->IsString()) {
    cef_value->SetString(value->GetStringValue());
  } else if (value->IsObject()) {
    // For objects, we'll convert them to a dictionary
    cef_value->SetDictionary(ConvertV8ObjectToDictionary(value));
  } else if (value->IsArray()) {
    // For arrays, we'll convert them to a list
    cef_value->SetList(ConvertV8ArrayToList(value));
  } else {
    // For unsupported types, set as null or handle as needed
    cef_value->SetNull();
  }
  return cef_value;
}

CefRefPtr<CefListValue> V8Handler::ConvertV8ArrayToList(CefRefPtr<CefV8Value> v8Array) {
  CefRefPtr<CefListValue> list = CefListValue::Create();
  int length = v8Array->GetArrayLength();
  for (int i = 0; i < length; ++i) {
    list->SetValue(i, ConvertV8ValueToCefValue(v8Array->GetValue(i)));
  }
  return list;
}

CefRefPtr<CefDictionaryValue> V8Handler::ConvertV8ObjectToDictionary(CefRefPtr<CefV8Value> v8Object) {
  CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
  std::vector<CefString> keys;
  v8Object->GetKeys(keys);
  for (const auto& key : keys) {
    dict->SetValue(key, ConvertV8ValueToCefValue(v8Object->GetValue(key)));
  }
  return dict;
}
