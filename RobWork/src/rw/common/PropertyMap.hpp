/********************************************************************************
 * Copyright 2009 The Robotics Group, The Maersk Mc-Kinney Moller Institute, 
 * Faculty of Engineering, University of Southern Denmark 
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ********************************************************************************/


#ifndef RW_COMMON_PROPERTYMAP_HPP
#define RW_COMMON_PROPERTYMAP_HPP

/**
 * @file PropertyMap.hpp
 */

#include "PropertyBase.hpp"
#include "Property.hpp"
#include "StringUtil.hpp"
#include "macros.hpp"

#include <set>
#include <memory>

namespace rw { namespace common {

    /** @addtogroup common */
    /*@{*/

    /**
     * @brief Container for a collection of Property Objects
     *
     * This container is used to bind various user information to for example
     * Frame.
     *
     * Example: Getting a string property with ID "Camera" from a frame
     *
     * \code
     * const std::string* ptr = frame.getPropertyMap().getPtr<std::string>("Camera");
     * if (ptr) {
     *    std::cout << "Property 'Camera' has value " << *ptr << "\n";
     * }
     * \endcode
     *
     */
    class PropertyMap {
    public:

        //! @brief smart pointer type
        typedef rw::common::Ptr<PropertyMap> Ptr;

        /**
         * @brief Constructor
         */
        PropertyMap();

        /**
         * @brief constructor
         * @param name [in] name of this propertymap
         */
        PropertyMap(std::string name):_name(name){};

        /**
         * @brief Destructor
         */
        ~PropertyMap();

        /**
           @brief Copy constructor.
        */
        PropertyMap(const PropertyMap& other);

        /**
           @brief Assignment operator.
        */
        PropertyMap& operator=(const PropertyMap& other);

        /**
           @brief swap operator.
        */
        void swap(PropertyMap& other);

        /**
         * @brief get the name of this propertymap
         * @return name of this propertymap
         */
        const std::string& getName(){ return _name; };

        /**
         * @brief Set the value of a property
         *
         * If a property with the given identifier cannot be found, a new
         * property with no description is created and inserted.
         *
         * @param identifier [in] the property identifier
         * @param value [in] the new value
         */
        template<class T>
        void set(const std::string& identifier, const T& value)
        {
            Property<T>* prop = findProperty<T>(identifier);
            if (prop)
                prop->setValue(value);
            else
                add(identifier, "", value);
        }

        /**
           @brief Add a property to the map.

           @param identifier [in] Property identifier.

           @param description [in] Property description.

           @param value [in] Property value.

           @return The property if added or NULL if the identifier is already in
           use.
        */
        template <typename T>
        rw::common::Ptr<Property<T> > add(const std::string& identifier,
                         const std::string& description,
                         const T& value)
        {
            rw::common::Ptr<Property<T> > property(new Property<T>(identifier, description, value));

            const bool ok = insert(property);
            if (ok)
                return property;
            else
                return NULL;
        }


        /**
         * @brief Adds a property to the map
         *
         * @param property [in] Property to add
         *
         * @return True if added, false if property already exists.
         */
        bool add(PropertyBasePtr property);

        /**
         * @brief Get the value of a property or NULL if no such property.
         *
         * If a property of the given identifier and type cannot be found, the
         * method returns NULL.
         *
         * @param identifier [in] the identifier of the property
         *
         * @return the value of the property
         */
        template<class T>
        T* getPtr(const std::string& identifier)
        {
            Property<T>* prop = findProperty<T>(identifier);
            if (prop)
                return &prop->getValue();
            else
                return NULL;
        }

        /**
         * @brief Get the value of a property or NULL if no such property.
         *
         * If a property of the given identifier and type cannot be found, the
         * method returns NULL.
         *
         * @param identifier [in] the identifier of the property
         *
         * @return the value of the property
         */
        template<class T>
        const T* getPtr(const std::string& identifier) const
        {
            // Forward to non-const method.
            return const_cast<PropertyMap*>(this)->getPtr<T>(identifier);
        }

        /**
         * @brief Get the value of a property
         *
         * If a property of the given identifier and type cannot be found, the
         * method throws an exception
         *
         * @param identifier [in] the identifier of the property
         *
         * @return the value of the property
         */
        template<class T>
        T& get(const std::string& identifier)
        {
            T* p = getPtr<T>(identifier);
            if (!p) {
                RW_THROW(
                    "Property "
                    << StringUtil::quote(identifier)
                    << " could not be found");
            }
            return *p;
        }

        /**
         * @brief Get the value of a property
         *
         * If a property of the given identifier and type cannot be found, the
         * method throws an exception
         *
         * @param identifier [in] the identifier of the property
         *
         * @return the value of the property
         */
        template<class T>
        const T& get(const std::string& identifier) const
        {
            // Forward to non-const method.
            return const_cast<PropertyMap*>(this)->get<T>(identifier);
        }

        /**
         * @brief Get the value of a property if it exists.
         *
         * If a property of the given identifier and type cannot be found, the
         * method returns the default value \b defval.
         *
         * \b example
         * int iterations = map.get<int>("Iterations", 20);
         *
         * @param identifier [in] the identifier of the property
         * @param defval [in] the value that will be returned if property with
         * \b identifier is not found.
         * @return the value of the property if it exists, else \b defval is returned
         *
         */
        template<class T>
        T& get(const std::string& identifier, T& defval)
        {
            T* p = getPtr<T>(identifier);
            if (!p) {
            	return defval;
            }
            return *p;
        }

        /**
         * @brief Get the value of a property
         *
         * If a property of the given identifier and type cannot be found, the
         * method throws an exception
         *
         * \b example
         * int iterations = map.get<int>("Iterations", 20);
         *
         * @param identifier [in] the identifier of the property
         * @param defval [in] the value that will be returned if property with
         * \b identifier is not found.
         * @return the value of the property if it exists, else \b defval is returned
         *
         */
        template<class T>
        const T& get(const std::string& identifier, const T& defval) const
        {
            const T* p = getPtr<T>(identifier);
            if (!p) {
                return defval;
            }
            return *p;
        }


        /**
         * @brief True if a specific property exists
         *
         * @param identifier [in] The identifier of the property
         * @return true if the property exists
         */
        bool has(const std::string& identifier) const;

        /**
         * @brief The number of properties
         */
        size_t size() const;

        /**
         * @brief True iff the property map contains no properties.
         */
        bool empty() const;

        /**
         * @brief Remove a property
         *
         * @return true if the property was successfully removed.
         */
        bool erase(const std::string& identifier);

        // The following methods are rarely used and are therefore given longer
        // names. They more strongly expose the internal use of Property<T>.

        /**
         * @brief Find the property for an identifier.
         *
         * The method finds the Property<T> object having a given identifier. If
         * no property with that identifier exists or if the value of the
         * property is not of type T then NULL is returned.
         *
         * @param identifier [in] property identifier
         *
         * @return Property object with that identifier
         */
        template<class T>
        Property<T>* findProperty(const std::string& identifier)
        {
            return dynamic_cast<Property<T>*>(findPropertyBase(identifier));
        }

        /**
         * @brief Find the property for an identifier.
         *
         * The method finds the Property<T> object having a given identifier. If
         * no property with that identifier exists or if the value of the
         * property is not of type T then NULL is returned.
         *
         * @param identifier [in] property identifier
         *
         * @return Property object with that identifier
         */
        template<class T>
        const Property<T>* findProperty(const std::string& identifier) const
        {
            return dynamic_cast<const Property<T>*>(
                findPropertyBase(identifier));
        }

        /**
         * @brief Find the property base for an identifier.
         *
         * The find methods returns pointer to PropertyBase object or NULL if a
         * property base with that identifier does not exist.
         *
         * @param identifier [in] identifier for the property base to find.
         */
        PropertyBase* findPropertyBase(const std::string& identifier);

        /**
         * @brief Find the property base for an identifier.
         *
         * The find methods returns pointer to PropertyBase object or NULL if a
         * property base with that identifier does not exist.
         *
         * @param identifier [in] identifier for the property base to find.
         */
        const PropertyBase* findPropertyBase(const std::string& identifier) const;

        /*
         * functions we need
         * addPropertyListener( listener )
         * removePropertyListener( )
         * firePropertyChanged()
         * firePropertyErased()
         */

    private:
        struct CmpPropertyBase
        {
            bool operator()(
                const PropertyBasePtr a,
                const PropertyBasePtr b) const
            {
                return a->getIdentifier() < b->getIdentifier();
            }
        };

        typedef std::set<PropertyBasePtr, CmpPropertyBase> MapType;

    public:
        //! Iterator for const PropertyBasePtr
        typedef MapType::const_iterator iterator;
        typedef std::pair<iterator,iterator> Range;
        /**
           @brief Range of all PropertyBase* objects stored.

           Note that this low-level interface does permits the PropertyBase
           values to be modified even though the method itself is declared
           const.
        */
        std::pair<iterator, iterator> getProperties() const;


    private:
        bool insert(PropertyBasePtr property);

    private:
        MapType _properties;
        std::string _name;
    };

    /** @} */
}} // end namespaces

#endif // end include guard
