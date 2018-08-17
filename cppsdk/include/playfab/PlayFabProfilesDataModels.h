#pragma once

#ifndef DISABLE_PLAYFABENTITY_API

#include <playfab/PlayFabBaseModel.h>

// Intellisense-only includes
#ifndef _WIN32
#include <jsoncpp/json/value.h>
#endif
#ifdef _WIN32
#include <json/value.h>
#endif

namespace PlayFab
{
    namespace ProfilesModels
    {
        // Profiles Enums
        enum EffectType
        {
            EffectTypeAllow,
            EffectTypeDeny
        };

        inline void ToJsonEnum(const EffectType input, Json::Value& output)
        {
            if (input == EffectTypeAllow) output = Json::Value("Allow");
            if (input == EffectTypeDeny) output = Json::Value("Deny");
        }
        inline void FromJsonEnum(const Json::Value& input, EffectType& output)
        {
            if (!input.isString()) return;
            const std::string& inputStr = input.asString();
            if (inputStr == "Allow") output = EffectTypeAllow;
            if (inputStr == "Deny") output = EffectTypeDeny;
        }

        enum EntityTypes
        {
            EntityTypestitle,
            EntityTypesmaster_player_account,
            EntityTypestitle_player_account,
            EntityTypescharacter,
            EntityTypesgroup,
            EntityTypesservice
        };

        inline void ToJsonEnum(const EntityTypes input, Json::Value& output)
        {
            if (input == EntityTypestitle) output = Json::Value("title");
            if (input == EntityTypesmaster_player_account) output = Json::Value("master_player_account");
            if (input == EntityTypestitle_player_account) output = Json::Value("title_player_account");
            if (input == EntityTypescharacter) output = Json::Value("character");
            if (input == EntityTypesgroup) output = Json::Value("group");
            if (input == EntityTypesservice) output = Json::Value("service");
        }
        inline void FromJsonEnum(const Json::Value& input, EntityTypes& output)
        {
            if (!input.isString()) return;
            const std::string& inputStr = input.asString();
            if (inputStr == "title") output = EntityTypestitle;
            if (inputStr == "master_player_account") output = EntityTypesmaster_player_account;
            if (inputStr == "title_player_account") output = EntityTypestitle_player_account;
            if (inputStr == "character") output = EntityTypescharacter;
            if (inputStr == "group") output = EntityTypesgroup;
            if (inputStr == "service") output = EntityTypesservice;
        }

        enum OperationTypes
        {
            OperationTypesCreated,
            OperationTypesUpdated,
            OperationTypesDeleted,
            OperationTypesNone
        };

        inline void ToJsonEnum(const OperationTypes input, Json::Value& output)
        {
            if (input == OperationTypesCreated) output = Json::Value("Created");
            if (input == OperationTypesUpdated) output = Json::Value("Updated");
            if (input == OperationTypesDeleted) output = Json::Value("Deleted");
            if (input == OperationTypesNone) output = Json::Value("None");
        }
        inline void FromJsonEnum(const Json::Value& input, OperationTypes& output)
        {
            if (!input.isString()) return;
            const std::string& inputStr = input.asString();
            if (inputStr == "Created") output = OperationTypesCreated;
            if (inputStr == "Updated") output = OperationTypesUpdated;
            if (inputStr == "Deleted") output = OperationTypesDeleted;
            if (inputStr == "None") output = OperationTypesNone;
        }

        // Profiles Classes
        struct EntityDataObject : public PlayFabBaseModel
        {
            Json::Value DataObject;
            std::string EscapedDataObject;
            std::string ObjectName;

            EntityDataObject() :
                PlayFabBaseModel(),
                DataObject(),
                EscapedDataObject(),
                ObjectName()
            {}

            EntityDataObject(const EntityDataObject& src) :
                PlayFabBaseModel(),
                DataObject(src.DataObject),
                EscapedDataObject(src.EscapedDataObject),
                ObjectName(src.ObjectName)
            {}

            ~EntityDataObject() = default;

            void FromJson(Json::Value& input) override
            {
                DataObject = input["DataObject"];
                FromJsonUtilS(input["EscapedDataObject"], EscapedDataObject);
                FromJsonUtilS(input["ObjectName"], ObjectName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                output["DataObject"] = DataObject;
                Json::Value each_EscapedDataObject; ToJsonUtilS(EscapedDataObject, each_EscapedDataObject); output["EscapedDataObject"] = each_EscapedDataObject;
                Json::Value each_ObjectName; ToJsonUtilS(ObjectName, each_ObjectName); output["ObjectName"] = each_ObjectName;
                return output;
            }
        };

        struct EntityKey : public PlayFabBaseModel
        {
            std::string Id;
            Boxed<EntityTypes> Type;
            std::string TypeString;

            EntityKey() :
                PlayFabBaseModel(),
                Id(),
                Type(),
                TypeString()
            {}

            EntityKey(const EntityKey& src) :
                PlayFabBaseModel(),
                Id(src.Id),
                Type(src.Type),
                TypeString(src.TypeString)
            {}

            ~EntityKey() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["Id"], Id);
                FromJsonUtilE(input["Type"], Type);
                FromJsonUtilS(input["TypeString"], TypeString);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Id; ToJsonUtilS(Id, each_Id); output["Id"] = each_Id;
                Json::Value each_Type; ToJsonUtilE(Type, each_Type); output["Type"] = each_Type;
                Json::Value each_TypeString; ToJsonUtilS(TypeString, each_TypeString); output["TypeString"] = each_TypeString;
                return output;
            }
        };

        struct EntityPermissionStatement : public PlayFabBaseModel
        {
            std::string Action;
            std::string Comment;
            Json::Value Condition;
            EffectType Effect;
            Json::Value Principal;
            std::string Resource;

            EntityPermissionStatement() :
                PlayFabBaseModel(),
                Action(),
                Comment(),
                Condition(),
                Effect(),
                Principal(),
                Resource()
            {}

            EntityPermissionStatement(const EntityPermissionStatement& src) :
                PlayFabBaseModel(),
                Action(src.Action),
                Comment(src.Comment),
                Condition(src.Condition),
                Effect(src.Effect),
                Principal(src.Principal),
                Resource(src.Resource)
            {}

            ~EntityPermissionStatement() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["Action"], Action);
                FromJsonUtilS(input["Comment"], Comment);
                Condition = input["Condition"];
                FromJsonEnum(input["Effect"], Effect);
                Principal = input["Principal"];
                FromJsonUtilS(input["Resource"], Resource);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Action; ToJsonUtilS(Action, each_Action); output["Action"] = each_Action;
                Json::Value each_Comment; ToJsonUtilS(Comment, each_Comment); output["Comment"] = each_Comment;
                output["Condition"] = Condition;
                Json::Value each_Effect; ToJsonEnum(Effect, each_Effect); output["Effect"] = each_Effect;
                output["Principal"] = Principal;
                Json::Value each_Resource; ToJsonUtilS(Resource, each_Resource); output["Resource"] = each_Resource;
                return output;
            }
        };

        struct EntityProfileFileMetadata : public PlayFabBaseModel
        {
            std::string Checksum;
            std::string FileName;
            time_t LastModified;
            Int32 Size;

            EntityProfileFileMetadata() :
                PlayFabBaseModel(),
                Checksum(),
                FileName(),
                LastModified(),
                Size()
            {}

            EntityProfileFileMetadata(const EntityProfileFileMetadata& src) :
                PlayFabBaseModel(),
                Checksum(src.Checksum),
                FileName(src.FileName),
                LastModified(src.LastModified),
                Size(src.Size)
            {}

            ~EntityProfileFileMetadata() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["Checksum"], Checksum);
                FromJsonUtilS(input["FileName"], FileName);
                FromJsonUtilT(input["LastModified"], LastModified);
                FromJsonUtilP(input["Size"], Size);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Checksum; ToJsonUtilS(Checksum, each_Checksum); output["Checksum"] = each_Checksum;
                Json::Value each_FileName; ToJsonUtilS(FileName, each_FileName); output["FileName"] = each_FileName;
                Json::Value each_LastModified; ToJsonUtilT(LastModified, each_LastModified); output["LastModified"] = each_LastModified;
                Json::Value each_Size; ToJsonUtilP(Size, each_Size); output["Size"] = each_Size;
                return output;
            }
        };

        struct EntityProfileBody : public PlayFabBaseModel
        {
            Boxed<EntityKey> Entity;
            std::string EntityChain;
            std::map<std::string, EntityProfileFileMetadata> Files;
            std::string Language;
            std::map<std::string, EntityDataObject> Objects;
            std::list<EntityPermissionStatement> Permissions;
            Int32 VersionNumber;

            EntityProfileBody() :
                PlayFabBaseModel(),
                Entity(),
                EntityChain(),
                Files(),
                Language(),
                Objects(),
                Permissions(),
                VersionNumber()
            {}

            EntityProfileBody(const EntityProfileBody& src) :
                PlayFabBaseModel(),
                Entity(src.Entity),
                EntityChain(src.EntityChain),
                Files(src.Files),
                Language(src.Language),
                Objects(src.Objects),
                Permissions(src.Permissions),
                VersionNumber(src.VersionNumber)
            {}

            ~EntityProfileBody() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilS(input["EntityChain"], EntityChain);
                FromJsonUtilO(input["Files"], Files);
                FromJsonUtilS(input["Language"], Language);
                FromJsonUtilO(input["Objects"], Objects);
                FromJsonUtilO(input["Permissions"], Permissions);
                FromJsonUtilP(input["VersionNumber"], VersionNumber);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_EntityChain; ToJsonUtilS(EntityChain, each_EntityChain); output["EntityChain"] = each_EntityChain;
                Json::Value each_Files; ToJsonUtilO(Files, each_Files); output["Files"] = each_Files;
                Json::Value each_Language; ToJsonUtilS(Language, each_Language); output["Language"] = each_Language;
                Json::Value each_Objects; ToJsonUtilO(Objects, each_Objects); output["Objects"] = each_Objects;
                Json::Value each_Permissions; ToJsonUtilO(Permissions, each_Permissions); output["Permissions"] = each_Permissions;
                Json::Value each_VersionNumber; ToJsonUtilP(VersionNumber, each_VersionNumber); output["VersionNumber"] = each_VersionNumber;
                return output;
            }
        };

        struct GetEntityProfileRequest : public PlayFabRequestCommon
        {
            Boxed<bool> DataAsObject;
            EntityKey Entity;

            GetEntityProfileRequest() :
                PlayFabRequestCommon(),
                DataAsObject(),
                Entity()
            {}

            GetEntityProfileRequest(const GetEntityProfileRequest& src) :
                PlayFabRequestCommon(),
                DataAsObject(src.DataAsObject),
                Entity(src.Entity)
            {}

            ~GetEntityProfileRequest() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["DataAsObject"], DataAsObject);
                FromJsonUtilO(input["Entity"], Entity);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_DataAsObject; ToJsonUtilP(DataAsObject, each_DataAsObject); output["DataAsObject"] = each_DataAsObject;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                return output;
            }
        };

        struct GetEntityProfileResponse : public PlayFabResultCommon
        {
            Boxed<EntityProfileBody> Profile;

            GetEntityProfileResponse() :
                PlayFabResultCommon(),
                Profile()
            {}

            GetEntityProfileResponse(const GetEntityProfileResponse& src) :
                PlayFabResultCommon(),
                Profile(src.Profile)
            {}

            ~GetEntityProfileResponse() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Profile"], Profile);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Profile; ToJsonUtilO(Profile, each_Profile); output["Profile"] = each_Profile;
                return output;
            }
        };

        struct GetEntityProfilesRequest : public PlayFabRequestCommon
        {
            Boxed<bool> DataAsObject;
            std::list<EntityKey> Entities;

            GetEntityProfilesRequest() :
                PlayFabRequestCommon(),
                DataAsObject(),
                Entities()
            {}

            GetEntityProfilesRequest(const GetEntityProfilesRequest& src) :
                PlayFabRequestCommon(),
                DataAsObject(src.DataAsObject),
                Entities(src.Entities)
            {}

            ~GetEntityProfilesRequest() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["DataAsObject"], DataAsObject);
                FromJsonUtilO(input["Entities"], Entities);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_DataAsObject; ToJsonUtilP(DataAsObject, each_DataAsObject); output["DataAsObject"] = each_DataAsObject;
                Json::Value each_Entities; ToJsonUtilO(Entities, each_Entities); output["Entities"] = each_Entities;
                return output;
            }
        };

        struct GetEntityProfilesResponse : public PlayFabResultCommon
        {
            std::list<EntityProfileBody> Profiles;

            GetEntityProfilesResponse() :
                PlayFabResultCommon(),
                Profiles()
            {}

            GetEntityProfilesResponse(const GetEntityProfilesResponse& src) :
                PlayFabResultCommon(),
                Profiles(src.Profiles)
            {}

            ~GetEntityProfilesResponse() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Profiles"], Profiles);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Profiles; ToJsonUtilO(Profiles, each_Profiles); output["Profiles"] = each_Profiles;
                return output;
            }
        };

        struct GetGlobalPolicyRequest : public PlayFabRequestCommon
        {

            GetGlobalPolicyRequest() :
                PlayFabRequestCommon()
            {}

            GetGlobalPolicyRequest(const GetGlobalPolicyRequest&) :
                PlayFabRequestCommon()
            {}

            ~GetGlobalPolicyRequest() = default;

            void FromJson(Json::Value&) override
            {
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                return output;
            }
        };

        struct GetGlobalPolicyResponse : public PlayFabResultCommon
        {
            std::list<EntityPermissionStatement> Permissions;

            GetGlobalPolicyResponse() :
                PlayFabResultCommon(),
                Permissions()
            {}

            GetGlobalPolicyResponse(const GetGlobalPolicyResponse& src) :
                PlayFabResultCommon(),
                Permissions(src.Permissions)
            {}

            ~GetGlobalPolicyResponse() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Permissions"], Permissions);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Permissions; ToJsonUtilO(Permissions, each_Permissions); output["Permissions"] = each_Permissions;
                return output;
            }
        };

        struct SetEntityProfilePolicyRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            std::list<EntityPermissionStatement> Statements;

            SetEntityProfilePolicyRequest() :
                PlayFabRequestCommon(),
                Entity(),
                Statements()
            {}

            SetEntityProfilePolicyRequest(const SetEntityProfilePolicyRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                Statements(src.Statements)
            {}

            ~SetEntityProfilePolicyRequest() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Statements"], Statements);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Statements; ToJsonUtilO(Statements, each_Statements); output["Statements"] = each_Statements;
                return output;
            }
        };

        struct SetEntityProfilePolicyResponse : public PlayFabResultCommon
        {
            std::list<EntityPermissionStatement> Permissions;

            SetEntityProfilePolicyResponse() :
                PlayFabResultCommon(),
                Permissions()
            {}

            SetEntityProfilePolicyResponse(const SetEntityProfilePolicyResponse& src) :
                PlayFabResultCommon(),
                Permissions(src.Permissions)
            {}

            ~SetEntityProfilePolicyResponse() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Permissions"], Permissions);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Permissions; ToJsonUtilO(Permissions, each_Permissions); output["Permissions"] = each_Permissions;
                return output;
            }
        };

        struct SetGlobalPolicyRequest : public PlayFabRequestCommon
        {
            std::list<EntityPermissionStatement> Permissions;

            SetGlobalPolicyRequest() :
                PlayFabRequestCommon(),
                Permissions()
            {}

            SetGlobalPolicyRequest(const SetGlobalPolicyRequest& src) :
                PlayFabRequestCommon(),
                Permissions(src.Permissions)
            {}

            ~SetGlobalPolicyRequest() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Permissions"], Permissions);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Permissions; ToJsonUtilO(Permissions, each_Permissions); output["Permissions"] = each_Permissions;
                return output;
            }
        };

        struct SetGlobalPolicyResponse : public PlayFabResultCommon
        {

            SetGlobalPolicyResponse() :
                PlayFabResultCommon()
            {}

            SetGlobalPolicyResponse(const SetGlobalPolicyResponse&) :
                PlayFabResultCommon()
            {}

            ~SetGlobalPolicyResponse() = default;

            void FromJson(Json::Value&) override
            {
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                return output;
            }
        };

        struct SetProfileLanguageRequest : public PlayFabRequestCommon
        {
            Boxed<EntityKey> Entity;
            Int32 ExpectedVersion;
            std::string Language;

            SetProfileLanguageRequest() :
                PlayFabRequestCommon(),
                Entity(),
                ExpectedVersion(),
                Language()
            {}

            SetProfileLanguageRequest(const SetProfileLanguageRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                ExpectedVersion(src.ExpectedVersion),
                Language(src.Language)
            {}

            ~SetProfileLanguageRequest() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilP(input["ExpectedVersion"], ExpectedVersion);
                FromJsonUtilS(input["Language"], Language);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_ExpectedVersion; ToJsonUtilP(ExpectedVersion, each_ExpectedVersion); output["ExpectedVersion"] = each_ExpectedVersion;
                Json::Value each_Language; ToJsonUtilS(Language, each_Language); output["Language"] = each_Language;
                return output;
            }
        };

        struct SetProfileLanguageResponse : public PlayFabResultCommon
        {
            Boxed<OperationTypes> OperationResult;
            Boxed<Int32> VersionNumber;

            SetProfileLanguageResponse() :
                PlayFabResultCommon(),
                OperationResult(),
                VersionNumber()
            {}

            SetProfileLanguageResponse(const SetProfileLanguageResponse& src) :
                PlayFabResultCommon(),
                OperationResult(src.OperationResult),
                VersionNumber(src.VersionNumber)
            {}

            ~SetProfileLanguageResponse() = default;

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilE(input["OperationResult"], OperationResult);
                FromJsonUtilP(input["VersionNumber"], VersionNumber);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_OperationResult; ToJsonUtilE(OperationResult, each_OperationResult); output["OperationResult"] = each_OperationResult;
                Json::Value each_VersionNumber; ToJsonUtilP(VersionNumber, each_VersionNumber); output["VersionNumber"] = each_VersionNumber;
                return output;
            }
        };

    }
}

#endif
