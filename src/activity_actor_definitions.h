#pragma once
#ifndef CATA_SRC_ACTIVITY_ACTOR_DEFINITIONS_H
#define CATA_SRC_ACTIVITY_ACTOR_DEFINITIONS_H

#include "activity_actor.h"

#include "coordinates.h"
#include "item_handling_util.h"
#include "item_location.h"
#include "memory_fast.h"
#include "optional.h"
#include "pickup_token.h"
#include "point.h"
#include "type_id.h"
#include "units_energy.h"

class Creature;
class vehicle;

class aim_activity_actor : public activity_actor
{
    private:
        cata::optional<item> fake_weapon;
        units::energy bp_cost_per_shot = 0_J;
        std::vector<tripoint> fin_trajectory;

    public:
        bool first_turn = true;
        std::string action = "";
        int aif_duration = 0; // Counts aim-and-fire duration
        bool aiming_at_critter = false; // Whether aiming at critter or a tile
        bool snap_to_target = false;
        bool shifting_view = false;
        tripoint initial_view_offset;
        /** Target UI requested to abort aiming */
        bool aborted = false;
        /** RELOAD_AND_SHOOT weapon is kept loaded by the activity */
        bool loaded_RAS_weapon = false;
        /**
         * Target UI requested to abort aiming and reload weapon
         * Implies aborted = true
         */
        bool reload_requested = false;
        /**
         * A friendly creature may enter line of fire during aim-and-shoot,
         * and that generates a warning to proceed/abort. If player decides to
         * proceed, that creature is saved in this vector to prevent the same warning
         * from popping up on the following turn(s).
         */
        std::vector<weak_ptr_fast<Creature>> acceptable_losses;

        aim_activity_actor();

        /** Aiming wielded gun */
        static aim_activity_actor use_wielded();

        /** Aiming fake gun provided by a bionic */
        static aim_activity_actor use_bionic( const item &fake_gun, const units::energy &cost_per_shot );

        /** Aiming fake gun provided by a mutation */
        static aim_activity_actor use_mutation( const item &fake_gun );

        activity_id get_type() const override {
            return activity_id( "ACT_AIM" );
        }

        void start( player_activity &act, Character &who ) override;
        void do_turn( player_activity &act, Character &who ) override;
        void finish( player_activity &act, Character &who ) override;
        void canceled( player_activity &act, Character &who ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<aim_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );

        item *get_weapon();
        void restore_view();
        // Load/unload a RELOAD_AND_SHOOT weapon
        bool load_RAS_weapon();
        void unload_RAS_weapon();
};

class autodrive_activity_actor : public activity_actor
{
    private:
        vehicle *player_vehicle = nullptr;

    public:
        autodrive_activity_actor() = default;

        activity_id get_type() const override {
            return activity_id( "ACT_AUTODRIVE" );
        }

        void start( player_activity &act, Character & ) override;
        void do_turn( player_activity &, Character & ) override;
        void canceled( player_activity &, Character & ) override;
        void finish( player_activity &act, Character & ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<autodrive_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class dig_activity_actor : public activity_actor
{
    private:
        int moves_total;
        /** location of the dig **/
        tripoint location;
        std::string result_terrain;
        tripoint byproducts_location;
        int byproducts_count;
        std::string byproducts_item_group;

        /**
         * Returns true if @p other and `this` are "equivalent" in the sense that
         *  `this` can be resumed instead of starting @p other.
         */
        bool equivalent_activity( const dig_activity_actor &other ) const {
            return  location == other.location &&
                    result_terrain == other.result_terrain &&
                    byproducts_location == other.byproducts_location &&
                    byproducts_count == other.byproducts_count &&
                    byproducts_item_group == other.byproducts_item_group;
        }

        /**
         * @pre @p other is a `dig_activity_actor`
         */
        bool can_resume_with_internal( const activity_actor &other, const Character & ) const override {
            const dig_activity_actor &d_actor = static_cast<const dig_activity_actor &>( other );
            return equivalent_activity( d_actor );
        }

    public:
        dig_activity_actor(
            int dig_moves, const tripoint &dig_loc,
            const std::string &resulting_ter, const tripoint &dump_loc,
            int dump_spawn_count, const std::string &dump_item_group
        ):
            moves_total( dig_moves ), location( dig_loc ),
            result_terrain( resulting_ter ),
            byproducts_location( dump_loc ),
            byproducts_count( dump_spawn_count ),
            byproducts_item_group( dump_item_group ) {}

        activity_id get_type() const override {
            return activity_id( "ACT_DIG" );
        }

        void start( player_activity &act, Character & ) override;
        void do_turn( player_activity &, Character & ) override;
        void finish( player_activity &act, Character &who ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<dig_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class dig_channel_activity_actor : public activity_actor
{
    private:
        int moves_total;
        /** location of the dig **/
        tripoint location;
        std::string result_terrain;
        tripoint byproducts_location;
        int byproducts_count;
        std::string byproducts_item_group;

        /**
         * Returns true if @p other and `this` are "equivalent" in the sense that
         *  `this` can be resumed instead of starting @p other.
         */
        bool equivalent_activity( const dig_channel_activity_actor &other ) const {
            return  location == other.location &&
                    result_terrain == other.result_terrain &&
                    byproducts_location == other.byproducts_location &&
                    byproducts_count == other.byproducts_count &&
                    byproducts_item_group == other.byproducts_item_group;
        }

        /**
         * @pre @p other is a `dig_activity_actor`
         */
        bool can_resume_with_internal( const activity_actor &other, const Character & ) const override {
            const dig_channel_activity_actor &dc_actor = static_cast<const dig_channel_activity_actor &>
                    ( other );
            return equivalent_activity( dc_actor );
        }

    public:
        dig_channel_activity_actor(
            int dig_moves, const tripoint &dig_loc,
            const std::string &resulting_ter, const tripoint &dump_loc,
            int dump_spawn_count, const std::string &dump_item_group
        ):
            moves_total( dig_moves ), location( dig_loc ),
            result_terrain( resulting_ter ),
            byproducts_location( dump_loc ),
            byproducts_count( dump_spawn_count ),
            byproducts_item_group( dump_item_group ) {}

        activity_id get_type() const override {
            return activity_id( "ACT_DIG_CHANNEL" );
        }

        void start( player_activity &act, Character & ) override;
        void do_turn( player_activity &, Character & ) override;
        void finish( player_activity &act, Character &who ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<dig_channel_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class disassemble_activity_actor : public activity_actor
{
    private:
        std::vector<iuse_location> targets;
        tripoint_abs_ms pos;
        bool recursive = false;
        int initial_num_targets = 0;

    public:
        disassemble_activity_actor() = default;
        disassemble_activity_actor(
            std::vector<iuse_location> &&targets,
            tripoint_abs_ms pos,
            bool recursive
        ) : targets( std::move( targets ) ), pos( pos ), recursive( recursive ) {}
        ~disassemble_activity_actor() = default;

        activity_id get_type() const override {
            return activity_id( "ACT_DISASSEMBLE" );
        }

        void start( player_activity &act, Character &who ) override;
        void do_turn( player_activity &, Character & ) override {};
        void finish( player_activity &act, Character &who ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<disassemble_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );

        act_progress_message get_progress_message(
            const player_activity &, const Character & ) const override;

        bool try_start_single( player_activity &act, Character &who );
        int calc_num_targets() const;
};

class drop_activity_actor : public activity_actor
{
    private:
        std::list<pickup::act_item> items;
        bool force_ground = false;
        tripoint relpos;

    public:
        drop_activity_actor() = default;
        drop_activity_actor( Character &ch, const drop_locations &items,
                             bool force_ground, const tripoint &relpos );

        activity_id get_type() const override {
            return activity_id( "ACT_DROP" );
        }

        void start( player_activity &, Character & ) override;
        void do_turn( player_activity &, Character &who ) override;
        void finish( player_activity &, Character & ) override {};

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<drop_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class hacking_activity_actor : public activity_actor
{
    private:
        bool using_bionic = false;

    public:
        struct use_bionic {};

        hacking_activity_actor() = default;
        hacking_activity_actor( use_bionic );

        activity_id get_type() const override {
            return activity_id( "ACT_HACKING" );
        }

        void start( player_activity &act, Character &who ) override;
        void do_turn( player_activity &, Character & ) override {};
        void finish( player_activity &act, Character &who ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<hacking_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class migration_cancel_activity_actor : public activity_actor
{
    public:
        migration_cancel_activity_actor() = default;

        activity_id get_type() const override {
            return activity_id( "ACT_MIGRATION_CANCEL" );
        }

        void start( player_activity &, Character & ) override {};
        void do_turn( player_activity &act, Character &who ) override;
        void finish( player_activity &, Character & ) override {};

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<migration_cancel_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class move_items_activity_actor : public activity_actor
{
    private:
        std::vector<item_location> target_items;
        std::vector<int> quantities;
        bool to_vehicle;
        tripoint relative_destination;

    public:
        move_items_activity_actor( std::vector<item_location> target_items, std::vector<int> quantities,
                                   bool to_vehicle, tripoint relative_destination ) :
            target_items( target_items ), quantities( quantities ), to_vehicle( to_vehicle ),
            relative_destination( relative_destination ) {}

        activity_id get_type() const override {
            return activity_id( "ACT_MOVE_ITEMS" );
        }

        void start( player_activity &, Character & ) override {};
        void do_turn( player_activity &act, Character &who ) override;
        void finish( player_activity &, Character & ) override {};

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<move_items_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class toggle_gate_activity_actor : public activity_actor
{
    private:
        int moves_total;
        tripoint placement;

        /**
         * @pre @p other is a toggle_gate_activity_actor
         */
        bool can_resume_with_internal( const activity_actor &other, const Character & ) const override {
            const toggle_gate_activity_actor &og_actor = static_cast<const toggle_gate_activity_actor &>
                    ( other );
            return placement == og_actor.placement;
        }

    public:
        toggle_gate_activity_actor( int gate_moves, const tripoint &gate_placement ) :
            moves_total( gate_moves ), placement( gate_placement ) {}

        activity_id get_type() const override {
            return activity_id( "ACT_TOGGLE_GATE" );
        }

        void start( player_activity &act, Character & ) override;
        void do_turn( player_activity &, Character & ) override {};
        void finish( player_activity &act, Character & ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<toggle_gate_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class pickup_activity_actor : public activity_actor
{
    private:
        /** Target items and the quantities thereof */
        std::vector<pickup::pick_drop_selection> target_items;

        /**
         * Position of the character when the activity is started. This is
         * stored so that we can cancel the activity if the player moves
         * (e.g. if the player is in a moving vehicle). This should be null
         * if not grabbing from the ground.
         */
        cata::optional<tripoint> starting_pos;

    public:
        pickup_activity_actor( const std::vector<pickup::pick_drop_selection> &target_items,
                               const cata::optional<tripoint> &starting_pos )
            : target_items( target_items )
            , starting_pos( starting_pos ) {}

        activity_id get_type() const override {
            return activity_id( "ACT_PICKUP" );
        }

        void start( player_activity &, Character & ) override {};
        void do_turn( player_activity &act, Character &who ) override;
        void finish( player_activity &, Character & ) override {};

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<pickup_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class stash_activity_actor : public activity_actor
{
    private:
        std::list<pickup::act_item> items;
        tripoint relpos;

    public:
        stash_activity_actor() = default;
        stash_activity_actor( Character &ch, const drop_locations &items, const tripoint &relpos );

        activity_id get_type() const override {
            return activity_id( "ACT_STASH" );
        }

        void start( player_activity &, Character & ) override;
        void do_turn( player_activity &, Character &who ) override;
        void finish( player_activity &, Character & ) override {};

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<stash_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class throw_activity_actor : public activity_actor
{
    private:
        item_location target_loc;
        cata::optional<tripoint> blind_throw_from_pos;

    public:
        throw_activity_actor() = default;
        throw_activity_actor(
            item_location target_loc,
            cata::optional<tripoint> blind_throw_from_pos
        ) : target_loc( target_loc ),
            blind_throw_from_pos( blind_throw_from_pos ) {}
        ~throw_activity_actor() = default;

        activity_id get_type() const override {
            return activity_id( "ACT_THROW" );
        }

        void start( player_activity &, Character & ) override {};
        void do_turn( player_activity &act, Character &who ) override;
        void finish( player_activity &, Character & ) override {};

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<throw_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class wash_activity_actor : public activity_actor
{
    private:
        iuse_locations targets;
        int moves_total = 0;

    public:
        wash_activity_actor() = default;
        wash_activity_actor( const iuse_locations &targets, int moves_total ) :
            targets( targets ), moves_total( moves_total ) {};

        activity_id get_type() const override {
            return activity_id( "ACT_WASH" );
        }

        void start( player_activity &act, Character & ) override;
        void do_turn( player_activity &, Character & ) override {};
        void finish( player_activity &act, Character &who ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<wash_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

#endif // CATA_SRC_ACTIVITY_ACTOR_DEFINITIONS_H
