//
// Generated file, do not edit! Created by nedtool 5.6 from userMsg.msg.
//

#ifndef __USERMSG_M_H
#define __USERMSG_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>userMsg.msg:19</tt> by nedtool.
 * <pre>
 * //
 * // TODO generated message class
 * //
 * packet UserMsg
 * {
 *     \@customize(true);  // see the generated C++ header for more info
 *     int someField;
 *     string msg;
 * }
 * </pre>
 *
 * UserMsg_Base is only useful if it gets subclassed, and UserMsg is derived from it.
 * The minimum code to be written for UserMsg is the following:
 *
 * <pre>
 * class UserMsg : public UserMsg_Base
 * {
 *   private:
 *     void copy(const UserMsg& other) { ... }

 *   public:
 *     UserMsg(const char *name=nullptr, short kind=0) : UserMsg_Base(name,kind) {}
 *     UserMsg(const UserMsg& other) : UserMsg_Base(other) {copy(other);}
 *     UserMsg& operator=(const UserMsg& other) {if (this==&other) return *this; UserMsg_Base::operator=(other); copy(other); return *this;}
 *     virtual UserMsg *dup() const override {return new UserMsg(*this);}
 *     // ADD CODE HERE to redefine and implement pure virtual functions from UserMsg_Base
 * };
 * </pre>
 *
 * The following should go into a .cc (.cpp) file:
 *
 * <pre>
 * Register_Class(UserMsg)
 * </pre>
 */
class UserMsg_Base : public ::omnetpp::cPacket
{
  protected:
    int someField;
    ::omnetpp::opp_string msg;

  private:
    void copy(const UserMsg_Base& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const UserMsg_Base&);
    // make constructors protected to avoid instantiation
    UserMsg_Base(const UserMsg_Base& other);
    // make assignment operator protected to force the user override it
    UserMsg_Base& operator=(const UserMsg_Base& other);

  public:
    UserMsg_Base(const char *name=nullptr, short kind=0);
    virtual ~UserMsg_Base();
    virtual UserMsg_Base *dup() const override {return new UserMsg_Base (*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getSomeField() const;
    virtual void setSomeField(int someField);
    virtual const char * getMsg() const;
    virtual void setMsg(const char * msg);
};


#endif // ifndef __USERMSG_M_H
