// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2009 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software 
 * Foundation.  See file COPYING.
 * 
 */

#ifndef CEPH_AUTHCLIENTHANDLER_H
#define CEPH_AUTHCLIENTHANDLER_H


#include "auth/Auth.h"
#include "common/lock_policy.h"
#include "common/lock_shared_mutex.h"

class CephContext;
struct MAuthReply;
template<ceph::LockPolicy> class RotatingKeyRing;

template<ceph::LockPolicy lock_policy>
class AuthClientHandler {
protected:
  CephContext *cct;
  EntityName name;
  uint64_t global_id;
  uint32_t want;
  uint32_t have;
  uint32_t need;
  mutable SharedMutexT<lock_policy> lock;

public:
  explicit AuthClientHandler(CephContext *cct_)
    : cct(cct_), global_id(0), want(CEPH_ENTITY_TYPE_AUTH), have(0), need(0),
      lock{SharedMutex<lock_policy>::create("AuthClientHandler::lock")}
  {}
  virtual ~AuthClientHandler() {}

  void init(const EntityName& n) { name = n; }
  
  void set_want_keys(__u32 keys) {
    std::unique_lock l{lock};
    want = keys | CEPH_ENTITY_TYPE_AUTH;
    validate_tickets();
  }

  virtual int get_protocol() const = 0;

  virtual void reset() = 0;
  virtual void prepare_build_request() = 0;
  virtual int build_request(bufferlist& bl) const = 0;
  virtual int handle_response(int ret, bufferlist::const_iterator& iter) = 0;
  virtual bool build_rotating_request(bufferlist& bl) const = 0;

  virtual AuthAuthorizer *build_authorizer(uint32_t service_id) const = 0;

  virtual bool need_tickets() = 0;

  virtual void set_global_id(uint64_t id) = 0;

  static AuthClientHandler<lock_policy>*
  create(CephContext *cct, int proto, RotatingKeyRing<lock_policy> *rkeys);
protected:
  virtual void validate_tickets() = 0;
};

#endif

