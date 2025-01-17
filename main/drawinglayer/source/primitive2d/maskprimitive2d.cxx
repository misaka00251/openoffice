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
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
	namespace primitive2d
	{
		MaskPrimitive2D::MaskPrimitive2D(
			const basegfx::B2DPolyPolygon& rMask,
			const Primitive2DSequence& rChildren)
		:	GroupPrimitive2D(rChildren),
			maMask(rMask)
		{
		}

		bool MaskPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
		{
			if(GroupPrimitive2D::operator==(rPrimitive))
			{
				const MaskPrimitive2D& rCompare = static_cast< const MaskPrimitive2D& >(rPrimitive);

				return (getMask() == rCompare.getMask());
			}

			return false;
		}

		basegfx::B2DRange MaskPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
		{
			return getMask().getB2DRange();
		}

		// provide unique ID
		ImplPrimitrive2DIDBlock(MaskPrimitive2D, PRIMITIVE2D_ID_MASKPRIMITIVE2D)

	} // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim: set noet sw=4 ts=4: */
