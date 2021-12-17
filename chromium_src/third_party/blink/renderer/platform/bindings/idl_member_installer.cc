/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "src/third_party/blink/renderer/platform/bindings/idl_member_installer.cc"

#include "base/strings/string_piece.h"
#include "third_party/blink/public/common/features.h"

namespace blink {

namespace bindings {

constexpr char kConnection[] = "connection";

// static
void IDLMemberInstaller::BraveInstallAttributes(
    v8::Isolate* isolate,
    const DOMWrapperWorld& world,
    v8::Local<v8::Template> instance_template,
    v8::Local<v8::Template> prototype_template,
    v8::Local<v8::Template> interface_template,
    v8::Local<v8::Signature> signature,
    base::span<const AttributeConfig> configs) {
  for (const auto& config : configs) {
    if (!base::FeatureList::IsEnabled(
            blink::features::kNavigatorConnectionAttribute) &&
        base::StringPiece(config.name).compare(kConnection) == 0) {
      continue;
    }
    InstallAttribute(isolate, world, instance_template, prototype_template,
                     interface_template, signature, config);
  }
}

// static
void IDLMemberInstaller::BraveInstallAttributes(
    v8::Isolate* isolate,
    const DOMWrapperWorld& world,
    v8::Local<v8::Object> instance_object,
    v8::Local<v8::Object> prototype_object,
    v8::Local<v8::Object> interface_object,
    v8::Local<v8::Signature> signature,
    base::span<const AttributeConfig> configs) {
  IDLMemberInstaller::InstallAttributes(isolate, world, instance_object,
                                        prototype_object, interface_object,
                                        signature, configs);
}

}  // namespace bindings

}  // namespace blink
