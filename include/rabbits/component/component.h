/*
 *  This file is part of Rabbits
 *  Copyright (C) 2015  Clement Deschamps and Luc Michel
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _UTILS_COMPONENT_COMPONENT_H
#define _UTILS_COMPONENT_COMPONENT_H

#include <map>
#include <vector>
#include <sstream>
#include <systemc>

#include "component_base.h"
#include "rabbits/rabbits_exception.h"

class Component : public ComponentBase {
protected:
    IrqPool<IrqIn> m_irqs_in;
    IrqPool<IrqOut> m_irqs_out;

    std::map<std::string, ComponentBase*> m_comp_children;
    std::map<std::string, SlaveIface*> m_slave_children;
    std::map<std::string, MasterIface*> m_master_children;

    virtual void declare_component(std::string id, ComponentBase& c)
    {
        m_comp_children[id] = &c;
    }

    virtual void declare_slave(std::string id, SlaveIface& s)
    {
        m_slave_children[id] = &s;
        m_comp_children[id] = &(s.get_component());
    }

    virtual void declare_master(std::string id, MasterIface& s)
    { 
        m_master_children[id] = &s;
        m_comp_children[id] = &(s.get_component());
    }

    virtual void declare_irq_in(std::string id, sc_core::sc_in<bool> & irq) {
        m_irqs_in.add(new IrqIn(id, irq));
    }

    virtual void declare_irq_in(IrqIn *irq) {
        m_irqs_in.add(irq);
    }

    virtual void declare_vector_irq_in(std::string id, sc_core::sc_vector<sc_core::sc_in<bool> > & v) {
        sc_core::sc_vector<sc_core::sc_in<bool> >::iterator it;
        int i;

        for (it = v.begin(), i = 0; it != v.end(); it++, i++) {
            std::stringstream ss;
            ss << id << i;
            declare_irq_in(ss.str(), *it);
        }
    }

    virtual void declare_irq_out(std::string id, sc_core::sc_out<bool> & irq) {
        m_irqs_out.add(new IrqOut(id, irq));
    }

    virtual void declare_irq_out(IrqOut *irq) {
        m_irqs_out.add(irq);
    }

    virtual void declare_vector_irq_out(std::string id, sc_core::sc_vector<sc_core::sc_out<bool> > & v) {
        sc_core::sc_vector<sc_core::sc_out<bool> >::iterator it;
        int i;

        for (it = v.begin(), i = 0; it != v.end(); it++, i++) {
            std::stringstream ss;
            ss << id << i;
            declare_irq_out(ss.str(), *it);
        }
    }

    virtual void stub_not_connected_irqs() {
        HasIrqInIface::iterator iti;
        HasIrqOutIface::iterator ito;
        
        for (ito = m_irqs_out.begin(); ito != m_irqs_out.end(); ito++) {
            if (!ito->second->is_connected()) {
                sc_core::sc_out<bool> &p = ito->second->get_port();
                sc_core::sc_signal<bool> &s = * new sc_core::sc_signal<bool>;
                p(s);
            }
        }

        for (iti = m_irqs_in.begin(); iti != m_irqs_in.end(); iti++) {
            if (!iti->second->is_connected()) {
                sc_core::sc_in<bool> &p = iti->second->get_port();
                sc_core::sc_signal<bool> &s = * new sc_core::sc_signal<bool>;
                p(s);
            }
        }
    }

    virtual void before_end_of_elaboration() {
        stub_not_connected_irqs();
    }

public:
    Component(sc_core::sc_module_name name, const ComponentParameters &params)
        : ComponentBase(name, params) {}
    Component(sc_core::sc_module_name name) : ComponentBase(name) {}
    virtual ~Component() {}

    /* HasIrqInIface */
    virtual IrqIn& get_irq_in(std::string id) {
        return m_irqs_in[id];
    }

    virtual IrqIn& get_irq_in(unsigned int idx) {
        return m_irqs_in[idx];
    }

    virtual bool irq_in_exists(std::string id) {
        return m_irqs_in.exists(id);
    }

    virtual bool irq_in_exists(unsigned int idx) {
        return m_irqs_in.exists(idx);
    }

    virtual HasIrqInIface::iterator irqs_in_begin() {
        return m_irqs_in.begin();
    }

    virtual HasIrqInIface::iterator irqs_in_end() {
        return m_irqs_in.end();
    }

    virtual HasIrqInIface::const_iterator irqs_in_begin() const {
        return m_irqs_in.begin();
    }

    virtual HasIrqInIface::const_iterator irqs_in_end() const {
        return m_irqs_in.end();
    }

    /* HasIrqOutIface */
    virtual IrqOut& get_irq_out(std::string id) {
        return m_irqs_out[id];
    }

    virtual IrqOut& get_irq_out(unsigned int idx) {
        return m_irqs_out[idx];
    }

    virtual bool irq_out_exists(std::string id) {
        return m_irqs_out.exists(id);
    }

    virtual bool irq_out_exists(unsigned int idx) {
        return m_irqs_out.exists(idx);
    }

    virtual HasIrqOutIface::iterator irqs_out_begin() {
        return m_irqs_out.begin();
    }

    virtual HasIrqOutIface::iterator irqs_out_end() {
        return m_irqs_out.end();
    }

    virtual HasIrqOutIface::const_iterator irqs_out_begin() const {
        return m_irqs_out.begin();
    }

    virtual HasIrqOutIface::const_iterator irqs_out_end() const {
        return m_irqs_out.end();
    }

    /* HasChildCompIface */
    virtual bool child_component_exists(std::string id) const {
        return m_comp_children.find(id) != m_comp_children.end();
    }

    virtual ComponentBase& get_child_component(std::string id) {
        if (!child_component_exists(id)) {
            throw ComponentNotFoundException(id);
        }

        return *m_comp_children[id];
    }

    virtual component_iterator child_component_begin() {
        return m_comp_children.begin();
    }

    virtual component_iterator child_component_end() {
        return m_comp_children.end();
    }

    virtual const_component_iterator child_component_begin() const {
        return m_comp_children.begin();
    }

    virtual const_component_iterator child_component_end() const {
        return m_comp_children.end();
    }

    virtual bool child_slave_exists(std::string id) const {
        return m_slave_children.find(id) != m_slave_children.end();
    }

    virtual SlaveIface& get_child_slave(std::string id) {
        if (!child_slave_exists(id)) {
            throw ComponentNotFoundException(id);
        }

        return *m_slave_children[id];
    }

    virtual bool child_master_exists(std::string id) const {
        return m_master_children.find(id) != m_master_children.end();
    }

    virtual MasterIface& get_child_master(std::string id) {
        if (!child_master_exists(id)) {
            throw ComponentNotFoundException(id);
        }

        return *m_master_children[id];
    }

    virtual master_iterator child_master_begin() {
        return m_master_children.begin();
    }

    virtual master_iterator child_master_end() {
        return m_master_children.end();
    }

    virtual const_master_iterator child_master_begin() const {
        return m_master_children.begin();
    }

    virtual const_master_iterator child_master_end() const {
        return m_master_children.end();
    }

    virtual slave_iterator child_slave_begin() {
        return m_slave_children.begin();
    }

    virtual slave_iterator child_slave_end() {
        return m_slave_children.end();
    }

    virtual const_slave_iterator child_slave_begin() const {
        return m_slave_children.begin();
    }

    virtual const_slave_iterator child_slave_end() const {
        return m_slave_children.end();
    }

};

#endif
