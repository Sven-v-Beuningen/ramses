//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererLib/SceneStateInfo.h"

using namespace testing;

namespace ramses_internal
{
    class ASceneStateInfo : public testing::Test
    {
    protected:
        ASceneStateInfo()
            : guid(true)
        {
        }

        void checkSceneIsUnknown(const SceneId scene) const
        {
            EXPECT_FALSE(isSceneKnown(scene));
        }

        void checkSceneIsKnown(const SceneId scene) const
        {
            EXPECT_TRUE(isSceneKnown(scene));
        }

        void checkNumberOfKnownScenes(UInt32 expectedNumber) const
        {
            SceneIdVector knownScenes;
            sceneStateInfo.getKnownSceneIds(knownScenes);
            EXPECT_EQ(expectedNumber, knownScenes.size());
        }

        SceneId sceneId;
        Guid guid;
        SceneStateInfo sceneStateInfo;

    private:
        Bool isSceneKnown(const SceneId scene) const
        {
            SceneIdVector knownScenes;
            sceneStateInfo.getKnownSceneIds(knownScenes);
            return knownScenes.contains(scene);
        }

    };

    TEST_F(ASceneStateInfo, IsInitializedCorrectly)
    {
        checkNumberOfKnownScenes(0u);
    }

    TEST_F(ASceneStateInfo, CanAddScene)
    {

        sceneStateInfo.addScene(sceneId, guid);
        EXPECT_TRUE(sceneStateInfo.hasScene(sceneId));
        checkNumberOfKnownScenes(1u);
        checkSceneIsKnown(sceneId);
    }

    TEST_F(ASceneStateInfo, CanRemoveScene)
    {
        sceneStateInfo.addScene(sceneId, guid);
        checkNumberOfKnownScenes(1u);
        checkSceneIsKnown(sceneId);

        sceneStateInfo.removeScene(sceneId);
        EXPECT_FALSE(sceneStateInfo.hasScene(sceneId));
        checkNumberOfKnownScenes(0u);
        checkSceneIsUnknown(sceneId);
    }

    TEST_F(ASceneStateInfo, CanAddAndRemoveMultipleScenes)
    {
        SceneId sceneId2(5u);
        sceneStateInfo.addScene(sceneId, guid);
        checkNumberOfKnownScenes(1u);
        checkSceneIsKnown(sceneId);
        checkSceneIsUnknown(sceneId2);

        sceneStateInfo.addScene(sceneId2, guid);
        checkNumberOfKnownScenes(2u);
        checkSceneIsKnown(sceneId);
        checkSceneIsKnown(sceneId2);

        sceneStateInfo.removeScene(sceneId);
        EXPECT_FALSE(sceneStateInfo.hasScene(sceneId));
        checkNumberOfKnownScenes(1u);
        checkSceneIsUnknown(sceneId);
        checkSceneIsKnown(sceneId2);

        sceneStateInfo.removeScene(sceneId2);
        EXPECT_FALSE(sceneStateInfo.hasScene(sceneId2));
        checkNumberOfKnownScenes(0u);
        checkSceneIsUnknown(sceneId);
        checkSceneIsUnknown(sceneId2);
    }

    TEST_F(ASceneStateInfo, CanGetSceneState)
    {
        sceneStateInfo.addScene(sceneId, guid);
        EXPECT_EQ(ESceneState_Published, sceneStateInfo.getSceneState(sceneId));
    }

    TEST_F(ASceneStateInfo, CanSetSceneState)
    {
        sceneStateInfo.addScene(sceneId, guid);
        sceneStateInfo.setSceneState(sceneId, ESceneState_Subscribed);
        EXPECT_EQ(ESceneState_Subscribed, sceneStateInfo.getSceneState(sceneId));
    }

    TEST_F(ASceneStateInfo, CanGetClientGuid)
    {
        sceneStateInfo.addScene(sceneId, guid);
        EXPECT_EQ(guid, sceneStateInfo.getSceneClientGuid(sceneId));
    }
}
