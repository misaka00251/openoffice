/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "connectivity/sdbcx/VIndexColumn.hxx"
#include "TConnection.hxx"

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OIndexColumn::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
	if(isNew())
		return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VIndexColumnDescription");
	return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VIndexColumn");
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OIndexColumn::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
	::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);
	if(isNew())
		aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.IndexColumnDescription");
	else
		aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.IndexColumn");

	return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OIndexColumn::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
	Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
	const ::rtl::OUString* pSupported = aSupported.getConstArray();
	const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
	for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
		;

	return pSupported != pEnd;
}
// -----------------------------------------------------------------------------
OIndexColumn::OIndexColumn(sal_Bool _bCase) : OColumn(_bCase),	m_IsAscending(sal_True)
{
	construct();
}

// -------------------------------------------------------------------------
OIndexColumn::OIndexColumn(	sal_Bool _IsAscending,
							const ::rtl::OUString&	_Name,
							const ::rtl::OUString&	_TypeName,
							const ::rtl::OUString&	_DefaultValue,
							sal_Int32				_IsNullable,
							sal_Int32				_Precision,
							sal_Int32				_Scale,
							sal_Int32				_Type,
							sal_Bool				_IsAutoIncrement,
							sal_Bool				_IsRowVersion,
							sal_Bool				_IsCurrency,
							sal_Bool				_bCase
						) : OColumn(_Name,
							_TypeName,
							_DefaultValue,
                            ::rtl::OUString(),
							_IsNullable,
							_Precision,
							_Scale,
							_Type,
							_IsAutoIncrement,
							_IsRowVersion,
							_IsCurrency,
							_bCase)
						,	m_IsAscending(_IsAscending)
{
	construct();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OIndexColumn::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& SAL_CALL OIndexColumn::getInfoHelper()
{
	return *OIndexColumn_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -------------------------------------------------------------------------
void OIndexColumn::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;
	registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISASCENDING),	PROPERTY_ID_ISASCENDING,	nAttrib,&m_IsAscending,	::getBooleanCppuType());
}
// -----------------------------------------------------------------------------



