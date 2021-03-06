/** @file
 * NetLabel Message Functions
 *
 * Author: Paul Moore <paul@paul-moore.com>
 *
 */

/*
 * (c) Copyright Hewlett-Packard Development Company, L.P., 2006
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/types.h>

#include <libnetlabel.h>

#include "netlabel_internal.h"

/*
 * Allocation Functions
 */

/**
 * Free a NetLabel message
 * @param msg the NetLabel message
 *
 * Free the memory associated with a NetLabel message.
 *
 */
void nlbl_msg_free(nlbl_msg *msg)
{
	if (msg == NULL)
		return;
	nlmsg_free(msg);
}

/**
 * Create a new NetLabel message
 *
 * Creates a new NetLabel message and allocates space for both the Netlink and
 * Generic Netlink headers.
 *
 */
nlbl_msg *nlbl_msg_new(void)
{
	nlbl_msg *msg;
	void *msg_buf;

	msg = nlmsg_alloc();
	if (msg == NULL)
		goto msg_new_failure;

	msg_buf = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, 0, 0, 0, 0, 0);
	if (msg_buf == NULL)
		goto msg_new_failure;

	return msg;

msg_new_failure:
	nlbl_msg_free(msg);
	return NULL;
}

/*
 * Netlink Header Functions
 */

/**
 * Get the Netlink header from the NetLabel message
 * @param msg the NetLabel message
 *
 * Returns the Netlink header on success, or NULL on failure.
 *
 */
struct nlmsghdr *nlbl_msg_nlhdr(nlbl_msg *msg)
{
	if (msg == NULL)
		return NULL;
	return nlmsg_hdr(msg);
}

/**
 * Get the Generic Netlink header from the NetLabel message
 * @param msg the NetLabel message
 *
 * Returns the Generic Netlink header on success, or NULL on failure.
 *
 */
struct genlmsghdr *nlbl_msg_genlhdr(nlbl_msg *msg)
{
	struct nlmsghdr *nl_hdr;

	/* NOTE: nlbl_msg_nlhdr() can handle being passed a NULL msg */
	nl_hdr = nlbl_msg_nlhdr(msg);
	if (nl_hdr == NULL)
		return NULL;

	return genlmsg_hdr(nl_hdr);
}

/*
 * Netlink Message Functions
 */

/**
 * Return the nlmsgerr struct in the Netlink message
 * @param msg the NetLabel message
 *
 * Returns a pointer to the nlmsgerr struct in a NLMSG_ERROR Netlink message
 * or NULL on failure.
 *
 */
struct nlmsgerr *nlbl_msg_err(nlbl_msg *msg)
{
	struct nlmsghdr *nl_hdr;

	/* NOTE: nlbl_msg_nlhdr() can handle being passed a NULL msg */
	nl_hdr = nlbl_msg_nlhdr(msg);
	if (nl_hdr == NULL || nl_hdr->nlmsg_type != NLMSG_ERROR)
		return NULL;
	return nlmsg_data(nl_hdr);
}

/*
 * Netlink Attribute Functions
 */

/**
 * Get the head of the Netlink attributes from a NetLabel msg
 * @param msg the NetLabel message
 *
 * Returns the head of the Netlink attributes from a NetLabel message on
 * success, or NULL on failure.
 *
 */
struct nlattr *nlbl_attr_head(nlbl_msg *msg)
{
	struct genlmsghdr *genl_hdr;

	/* NOTE: nlbl_msg_genlhdr() can handle being passed a NULL msg */
	genl_hdr = nlbl_msg_genlhdr(msg);
	if (genl_hdr == NULL)
		return NULL;

	return genlmsg_attrdata(genl_hdr, 0);
}

/**
 * Find an attribute in a NetLabel message
 * @param msg the NetLabel message
 * @param nla_type the attribute type
 *
 * Search @msg looking for the @nla_type attribute.  Return the attribute on
 * success, or NULL on failure.
 *
 */
struct nlattr *nlbl_attr_find(nlbl_msg *msg, int nla_type)
{
	struct genlmsghdr *genl_hdr;
	struct nlattr *nla_head;

	/* NOTE: nlbl_msg_genlhdr() can handle being passed a NULL msg */
	genl_hdr = nlbl_msg_genlhdr(msg);
	if (genl_hdr == NULL)
		return NULL;

	nla_head = genlmsg_attrdata(genl_hdr, 0);
	if (nla_head == NULL)
		return NULL;

	return nla_find(nla_head, genlmsg_attrlen(genl_hdr, 0), nla_type);
}
