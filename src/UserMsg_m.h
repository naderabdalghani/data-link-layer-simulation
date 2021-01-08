//
// Generated file, do not edit! Created by nedtool 5.6 from UserMsg.msg.
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
 * Class generated from <tt>UserMsg.msg:1</tt> by nedtool.
 * <pre>
 * packet UserMsg
 * {
 *     \@customize(true);
 *     int type;	// 0-> from hub to nodes first time, 1-> message to node, 2-> ACK for node, 3-> NACk for node
 *     string payload;
 *     int line_nr;
 *     int line_expected;
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
    int type;
    ::omnetpp::opp_string payload;
    int line_nr;
    int line_expected;

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
    virtual ~UserMsg_Base();
    UserMsg_Base(const char *name=nullptr, short kind=0);
    virtual UserMsg_Base *dup() const override {return new UserMsg_Base(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getType() const;
    virtual void setType(int type);
    virtual const char * getPayload() const;
    virtual void setPayload(const char * payload);
    virtual int getLine_nr() const;
    virtual void setLine_nr(int line_nr);
    virtual int getLine_expected() const;
    virtual void setLine_expected(int line_expected);
};


#endif // ifndef __USERMSG_M_H

