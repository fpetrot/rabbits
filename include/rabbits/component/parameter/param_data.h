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

/**
 * @file param_data.h
 * @brief ParamDataBase and ParamData class declaration.
 */

#ifndef _UTILS_COMPONENT_PARAMETER_PARAM_DATA_H
#define _UTILS_COMPONENT_PARAMETER_PARAM_DATA_H

/**
 * @brief ParamData, base class.
 *
 * This class is internally used by parameters as data storage for their values.
 */
class ParamDataBase {
private:
    bool m_inited;

protected:
    ParamDataBase() : m_inited(false) {}
    void inited() { m_inited = true; }

public:
    virtual ~ParamDataBase() {}

    bool is_inited() const { return m_inited; }
};

/**
 * @brief ParamData.
 *
 * This class is internally used by parameters as data storage for their values.
 */
template <typename T>
class ParamData : public ParamDataBase {
protected:
    T m_data;

public:
    ParamData() {}
    explicit ParamData(const T& d) : m_data(d) { inited(); }
    virtual ~ParamData() {}

    /**
     * @brief Get the internal storage value.
     *
     * @return the internal storage value.
     */
    T get() const { return m_data; }

    /**
     * @brief Set the internal storage value.
     *
     * @param d the new internal storage value.
     */
    void set(const T &d) { m_data = d; inited(); }
};

#endif
